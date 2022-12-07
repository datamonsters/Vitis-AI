

#
# Copyright 2019 Xilinx Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import torch
from torch.autograd import Variable
import math

from nndct_shared.utils import NndctOption
from nndct_shared.quantization import quantize_tensors
from nndct_shared.quantization import maybe_get_quantizer
import pytorch_nndct.utils as py_utils
import torch.nn.functional as F
__all__ = ['GroupNorm']

class deephi_GroupNorm(torch.nn.modules.normalization.GroupNorm):
  r"""DeePhi group normalization operation, support float and double"""

  def __init__(self, *args, **kwards):
    super(deephi_GroupNorm, self).__init__(*args, **kwards)
    self.params_name = None
    self.node = None
    self.quant_mode, self.quantizer = maybe_get_quantizer()
    self.param_saved = False
    self.param_quantized = False

    
  def forward(self, input):
    if NndctOption.nndct_quant_off.value or not self.quantizer.configer.is_node_quantizable(self.node, lstm=False):
      return super().forward(input)
    
    params = []
    if self.weight is not None:
      params.append(self.weight)
    if self.bias is not None:
      params.append(self.bias)
    param_names = self.params_name[:len(params)]
    if len(params) != len(param_names):
      NndctScreenLogger().error(f"Parameter number in Instance operator error!")
      exit(2)

    qinput = quantize_tensors([input], self.node, tensor_type='input')[0]
  
    if (not self.param_quantized) and len(params) > 0:
      inplace = self.quantizer is not None and self.quantizer.inplace
      # quantize weights and bias
      if inplace:
        _ = quantize_tensors(
            params,
            self.node,
            tensor_names=param_names,
            tensor_type='param')
        qparams = [p for p in params]
      else:
        qparams = quantize_tensors(
            params,
            self.node,
            tensor_names=param_names,
            tensor_type='param')
      if not NndctOption.nndct_quant_off.value:
        self.param_quantized = True
    else:
      qparams = [p for p in params]

    output = torch.nn.functional.group_norm(
            qinput,
            self.num_groups,
            self.weight,
            self.bias,
            self.eps
        )

    # quantize output
    output = quantize_tensors([output], self.node)[0]
    return output

  def _check_input_dim(self, input):
    pass

 
@py_utils.register_quant_op
def GroupNorm(*args, **kwargs):
  quant_mode, _ = maybe_get_quantizer()
  if quant_mode == None:
    def _check_input_dim(self, input):
      pass
    import types
    nn = torch.nn.modules.normalization.GroupNorm(*args, **kwargs)
    
    nn._check_input_dim = types.MethodType(_check_input_dim, nn)
    return nn
  return deephi_GroupNorm(*args, **kwargs)
