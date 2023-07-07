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
#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vitis/ai/demo_accuracy.hpp>
#include <vitis/ai/nnpp/yolov3.hpp>
#include <vitis/ai/yolov3.hpp>
extern int g_last_frame_id;

std::string model_name;

namespace vitis {
namespace ai {

static std::map<std::string, std::vector<std::string>> yolo_map{
    {"yolov3_adas_pruned_0_9", {"car", "person", "cycle"}},
    {"yolov3_bdd",
     {"bike", "bus", "car", "motor", "person", "rider", "light", "sign",
      "train", "truck"}},
    {"yolov3_voc",
     {"aeroplane",   "bicycle", "bird",  "boat",      "bottle",
      "bus",         "car",     "cat",   "chair",     "cow",
      "diningtable", "dog",     "horse", "motorbike", "person",
      "pottedplant", "sheep",   "sofa",  "train",     "tvmonitor"}},
    {"yolov3_voc_tf",
     {"aeroplane",   "bicycle", "bird",  "boat",      "bottle",
      "bus",         "car",     "cat",   "chair",     "cow",
      "diningtable", "dog",     "horse", "motorbike", "person",
      "pottedplant", "sheep",   "sofa",  "train",     "tvmonitor"}}};

static std::vector<std::string> split(const std::string& s,
                                      const std::string& delim) {
  std::vector<std::string> elems;
  size_t pos = 0;
  size_t len = s.length();
  size_t delim_len = delim.length();
  if (delim_len == 0) return elems;
  while (pos < len) {
    int find_pos = s.find(delim, pos);
    if (find_pos < 0) {
      elems.push_back(s.substr(pos, len - pos));
      break;
    }
    elems.push_back(s.substr(pos, find_pos - pos));
    pos = find_pos + delim_len;
  }
  return elems;
}

struct Yolov3Acc : public AccThread {
  Yolov3Acc(std::string output_file)
      : AccThread(), of(output_file, std::ofstream::out) {
    dpu_result.frame_id = -1;
  }

  virtual ~Yolov3Acc() { of.close(); }

  static std::shared_ptr<Yolov3Acc> instance(std::string output_file) {
    static std::weak_ptr<Yolov3Acc> the_instance;
    std::shared_ptr<Yolov3Acc> ret;
    if (the_instance.expired()) {
      ret = std::make_shared<Yolov3Acc>(output_file);
      the_instance = ret;
    }
    ret = the_instance.lock();
    assert(ret != nullptr);
    return ret;
  }

  void process_result(DpuResultInfo dpu_result) {
    auto result = (YOLOv3Result*)dpu_result.result_ptr.get();
    for (auto& box : result->bboxes) {
      float xmin = box.x * dpu_result.w + 1;
      float ymin = box.y * dpu_result.h + 1;
      float xmax = (box.x + box.width) * dpu_result.w + 1;
      float ymax = (box.y + box.height) * dpu_result.h + 1;
      if (xmin < 0) xmin = 1;
      if (ymin < 0) ymin = 1;
      if (xmax > dpu_result.w) xmax = dpu_result.w;
      if (ymax > dpu_result.h) ymax = dpu_result.h;
      of << split(dpu_result.single_name, ".")[0] << " "
         << yolo_map[model_name][box.label] << " " << box.score << " " << xmin
         << " " << ymin << " " << xmax << " " << ymax << std::endl;
    }
  }

  virtual int run() override {
    if (g_last_frame_id == int(dpu_result.frame_id)) return -1;
    if (queue_->pop(dpu_result, std::chrono::milliseconds(50000)))
      process_result(dpu_result);
    return 0;
  }

  DpuResultInfo dpu_result;
  std::ofstream of;
};

}  // namespace ai
}  // namespace vitis

int main(int argc, char* argv[]) {
  model_name = argv[1];
  return vitis::ai::main_for_accuracy_demo(
      argc, argv,
      [&] { return vitis::ai::YOLOv3::create(model_name + "_acc"); },
      vitis::ai::Yolov3Acc::instance(argv[3]), 2);
}
