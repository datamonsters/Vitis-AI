/*
 * Copyright 2022-2023 Advanced Micro Devices Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "./ssd_detector.hpp"

#include <glog/logging.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <thread>
#include <tuple>

#include "vitis/ai/nnpp/apply_nms.hpp"
namespace vitis {
namespace ai {
namespace multitask {

using namespace cv;
using namespace std;

SSDdetector::SSDdetector(unsigned int num_classes,  // int background_label_id,
                         CodeType code_type, bool variance_encoded_in_target,
                         unsigned int keep_top_k,
                         const vector<float>& confidence_threshold,
                         unsigned int nms_top_k, float nms_threshold, float eta,
                         const vector<shared_ptr<vector<float>>>& priors,
                         float scale, bool clip)
    : num_classes_(num_classes),
      // background_label_id_(background_label_id),
      code_type_(code_type),
      variance_encoded_in_target_(variance_encoded_in_target),
      keep_top_k_(keep_top_k),
      confidence_threshold_(confidence_threshold),
      nms_top_k_(nms_top_k),
      nms_threshold_(nms_threshold),
      eta_(eta),
      priors_(priors),
      scale_(scale),
      clip_(clip) {
  num_priors_ = priors_.size();

  nms_confidence_ = *std::min_element(confidence_threshold_.begin() + 1,
                                      confidence_threshold_.end());
}

void SSDdetector::Detect(const std::map<uint32_t, SSDOutputInfo>& loc_infos,
                         const float* conf_data,
                         std::vector<VehicleResult>& result) {
  decoded_bboxes_.clear();

  //  __TIC__(Sort)
  unsigned int num_det = 0;
  vector<vector<int>> indices(num_classes_);
  vector<vector<pair<float, int>>> score_index_vec(num_classes_);

  // Get top_k scores (with corresponding indices).
  GetMultiClassMaxScoreIndexMT(conf_data, 1, num_classes_ - 1,
                               &score_index_vec);
  //  __TOC__(Sort)
  //  __TIC__(NMS)
  for (unsigned int c = 1; c < num_classes_; ++c) {
    // Perform NMS for one class
    ApplyOneClassNMS(loc_infos, conf_data, c, score_index_vec[c],
                     &(indices[c]));

    num_det += indices[c].size();
  }

  if (keep_top_k_ > 0 && num_det > keep_top_k_) {
    vector<tuple<float, int, int>> score_index_tuples;
    for (size_t label = 0; label < num_classes_; ++label) {
      const vector<int>& label_indices = indices[label];
      for (size_t j = 0; j < label_indices.size(); ++j) {
        auto idx = label_indices[j];
        auto score = conf_data[idx * num_classes_ + label];
        score_index_tuples.emplace_back(score, label, idx);
      }
    }

    // Keep top k results per image.
    std::sort(score_index_tuples.begin(), score_index_tuples.end(),
              [](const tuple<float, int, int>& lhs,
                 const tuple<float, int, int>& rhs) {
                return get<0>(lhs) > get<0>(rhs);
              });
    score_index_tuples.resize(keep_top_k_);

    indices.clear();
    indices.resize(num_classes_);
    for (auto& item : score_index_tuples) {
      indices[get<1>(item)].push_back(get<2>(item));
    }

    num_det = keep_top_k_;
  }

  //  __TOC__(NMS)

  //  __TIC__(Box)
  for (size_t label = 1; label < indices.size(); ++label) {
    for (size_t idx : indices[label]) {
      auto score = conf_data[idx * num_classes_ + label];
      auto& bbox = decoded_bboxes_[idx];
      VehicleResult res;
      res.label = label;
      res.score = score;
      res.x = bbox[0] - 0.5f * bbox[2];
      res.y = bbox[1] - 0.5f * bbox[3];
      res.width = bbox[2];
      res.height = bbox[3];
      res.angle = atan2(bbox[5], bbox[4]) * 180.0 / 3.1415926;
      result.emplace_back(res);
    }
  }

  //  __TOC__(Box)
}

// template void SSDdetector::Detect(const int* loc_data, const float*
// conf_data,
//                                  std::vector<VehicleResult>& result);
// template void SSDdetector::Detect(const int8_t* loc_data,
//                                  const float* conf_data,
//                                  std::vector<VehicleResult>& result);

void SSDdetector::ApplyOneClassNMS(
    const std::map<uint32_t, SSDOutputInfo>& bbox_layer_infos,
    const float* conf_data, int label,
    const vector<pair<float, int>>& score_index_vec, vector<int>* indices) {
  vector<size_t> results;
  vector<vector<float>> boxes;
  vector<float> scores;
  map<size_t, int> resultmap;
  // float adaptive_threshold = nms_threshold_;
  indices->clear();
  unsigned int i = 0;
  while (i < score_index_vec.size()) {
    const uint32_t idx = score_index_vec[i].second;
    if (decoded_bboxes_.find(idx) == decoded_bboxes_.end()) {
      for (auto it = bbox_layer_infos.begin(); it != bbox_layer_infos.end();
           ++it) {
        if (idx >= it->second.index_begin &&
            idx < it->second.index_begin + it->second.index_size) {
          DecodeBBox(it->second.ptr + (idx - it->second.index_begin) *
                                          it->second.bbox_single_size,
                     idx, it->second.scale, true);
          break;
        }
      }
    }

    boxes.push_back(decoded_bboxes_[idx]);
    scores.push_back(score_index_vec[i].first);
    resultmap[i] = idx;
    ++i;
  }

  applyNMS(boxes, scores, nms_threshold_, confidence_threshold_[label],
           results);
  for (auto& r : results) {
    indices->push_back(resultmap[r]);
  }
}
void SSDdetector::GetOneClassMaxScoreIndex(
    const float* conf_data, int label,
    vector<pair<float, int>>* score_index_vec) {
  //__TIC__(PUSH2)
  conf_data += label;
  for (int i = 0; i < num_priors_; ++i) {
    auto score = *conf_data;
    if (score > nms_confidence_) {
      score_index_vec->emplace_back(score, i);
    }
    conf_data += num_classes_;
  }
  //__TOC__(PUSH2)
  //__TIC__(SORT2)
  std::stable_sort(
      score_index_vec->begin(), score_index_vec->end(),
      [](const pair<float, int>& lhs, const pair<float, int>& rhs) {
        return lhs.first > rhs.first;
      });
  //__TOC__(SORT2)

  if (nms_top_k_ > 0 && nms_top_k_ < score_index_vec->size()) {
    score_index_vec->resize(nms_top_k_);
  }
}

void SSDdetector::GetMultiClassMaxScoreIndex(
    const float* conf_data, int start_label, int num_classes,
    vector<vector<pair<float, int>>>* score_index_vec) {
  for (auto i = start_label; i < start_label + num_classes; ++i) {
    GetOneClassMaxScoreIndex(conf_data, i, &((*score_index_vec)[i]));
  }
}

void SSDdetector::GetMultiClassMaxScoreIndexMT(
    const float* conf_data, int start_label, int num_classes,
    vector<vector<pair<float, int>>>* score_index_vec, int threads) {
  // CHECK_GT(threads, 0);
  int thread_classes = num_classes / threads;
  int last_thread_classes = num_classes % threads + thread_classes;

  vector<std::thread> workers;

  auto c = start_label;
  for (auto i = 0; i < threads - 1; ++i) {
    workers.emplace_back(&SSDdetector::GetMultiClassMaxScoreIndex, this,
                         conf_data, c, thread_classes, score_index_vec);
    c += thread_classes;
  }
  workers.emplace_back(&SSDdetector::GetMultiClassMaxScoreIndex, this,
                       conf_data, c, last_thread_classes, score_index_vec);

  for (auto& worker : workers)
    if (worker.joinable()) worker.join();
}

void SSDdetector::DecodeBBox(const int8_t* bbox_ptr, int idx, float scale,
                             bool normalized) {
  vector<float> bbox(6, 0);
  // scale bboxes
  transform(bbox_ptr, bbox_ptr + 6, bbox.begin(),
            std::bind2nd(multiplies<float>(), scale));
  for (int i = 0; i < 1; i++) {
    // LOG(INFO) << "lalalal========" << bbox[0] << " " << bbox[1] << " " <<
    // bbox[2] << " " << bbox[3] << " " << bbox[4];
  }
  auto& prior_bbox = priors_[idx];

  if (code_type_ == CodeType::CORNER) {
    if (variance_encoded_in_target_) {
      // variance is encoded in target, we simply need to add the offset
      // predictions.
      transform(bbox.begin(), bbox.end(), prior_bbox->begin(), bbox.begin(),
                plus<float>());
    } else {
      // variance is encoded in bbox, we need to scale the offset accordingly.
      transform(bbox.begin(), bbox.end(), prior_bbox->begin() + 6, bbox.begin(),
                multiplies<float>());
      transform(bbox.begin(), bbox.end(), prior_bbox->begin(), bbox.begin(),
                plus<float>());
    }
  } else if (code_type_ == CodeType::CENTER_SIZE) {
    float decode_bbox_center_x, decode_bbox_center_y;
    float decode_bbox_width, decode_bbox_height;
    if (variance_encoded_in_target_) {
      // variance is encoded in target, we simply need to retore the offset
      // predictions.
      decode_bbox_center_x = bbox[0] * (*prior_bbox)[10] + (*prior_bbox)[8];
      decode_bbox_center_y = bbox[1] * (*prior_bbox)[11] + (*prior_bbox)[9];
      decode_bbox_width = exp(bbox[2]) * (*prior_bbox)[10];
      decode_bbox_height = exp(bbox[3]) * (*prior_bbox)[11];
    } else {
      // variance is encoded in bbox, we need to scale the offset accordingly.
      decode_bbox_center_x =
          (*prior_bbox)[4] * bbox[0] * (*prior_bbox)[10] + (*prior_bbox)[8];
      decode_bbox_center_y =
          (*prior_bbox)[5] * bbox[1] * (*prior_bbox)[11] + (*prior_bbox)[9];
      decode_bbox_width = exp((*prior_bbox)[6] * bbox[2]) * (*prior_bbox)[10];
      decode_bbox_height = exp((*prior_bbox)[7] * bbox[3]) * (*prior_bbox)[11];
    }

    bbox[0] = decode_bbox_center_x - decode_bbox_width / 2.;
    bbox[1] = decode_bbox_center_y - decode_bbox_height / 2.;
    bbox[2] = decode_bbox_center_x + decode_bbox_width / 2.;
    bbox[3] = decode_bbox_center_y + decode_bbox_height / 2.;
  } else if (code_type_ == CodeType::CORNER_SIZE) {
    if (variance_encoded_in_target_) {
      // variance is encoded in target, we simply need to add the offset
      // predictions.
      bbox[0] *= (*prior_bbox)[10];
      bbox[1] *= (*prior_bbox)[11];
      bbox[2] *= (*prior_bbox)[10];
      bbox[3] *= (*prior_bbox)[11];
      transform(bbox.begin(), bbox.end(), prior_bbox->begin(), bbox.begin(),
                plus<float>());
    } else {
      // variance is encoded in bbox, we need to scale the offset accordingly.
      bbox[0] *= (*prior_bbox)[10];
      bbox[1] *= (*prior_bbox)[11];
      bbox[2] *= (*prior_bbox)[10];
      bbox[3] *= (*prior_bbox)[11];
      transform(bbox.begin(), bbox.end(), prior_bbox->begin() + 4, bbox.begin(),
                multiplies<float>());
      transform(bbox.begin(), bbox.end(), prior_bbox->begin(), bbox.begin(),
                plus<float>());
    }
  } else {
    LOG(FATAL) << "Unknown LocLossType.";
  }

  // bbox x,y,w,h
  bbox[0] = std::max(std::min(bbox[0], 1.f), 0.f);
  bbox[1] = std::max(std::min(bbox[1], 1.f), 0.f);
  bbox[2] = std::max(std::min(bbox[2], 1.f), 0.f);
  bbox[3] = std::max(std::min(bbox[3], 1.f), 0.f);

  bbox[0] = 0.5f * (bbox[0] + bbox[2]);
  bbox[1] = 0.5f * (bbox[1] + bbox[3]);

  bbox[2] = (bbox[2] - bbox[0]) * 2.0f;
  bbox[3] = (bbox[3] - bbox[1]) * 2.0f;

  decoded_bboxes_.emplace(idx, std::move(bbox));
}

}  // namespace multitask
}  // namespace ai
}  // namespace vitis
