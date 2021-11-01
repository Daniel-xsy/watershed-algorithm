### 主要内容

1. C语言基本知识复习
2. VScode安装gcc
3. 程序编译过程复习
4. C语言读取图片
5. 算法实现



### 总结

##### 1. Unable to start debugging. Unexpected GDB output from command

文件夹路径不能有中文

##### 2. 环境变量中的admin的用户变量和系统变量的区别

admin用户变量只对当前用户生效，系统变量对所有用户生效。则当前的环境变量相当于两个变量的叠加

参考https://blog.csdn.net/sxhlovehmm/article/details/44274633

##### 3. C语言编译过程

头文件只能引用一次，所以在头文件中固定加入以下内容

```c
/*头文件只能引用一次*/
#ifndef HEAD_FILE
#define HEAD_FILE

#endif
```

##### 4. pack(1)的作用

使编译器让`struct`中的变量强制连续排序，否则编译器会自动优化程序。比如使得此处的`char ch`实际占用4个字节，使得`int i`的地址能够被4整除

```c
#progma pack(1)
struct s {
char ch;
int i;
};
#pragma pack()
```

##### C语言中的指针



##### 分水岭算法Python实现

```python
import numpy as np

class Watershed(object):
   MASK = -2
   WSHD = 0
   INIT = -1
   INQE = -3

   def __init__(self, levels = 256):
      self.levels = levels

   # Neighbour (coordinates of) pixels, including the given pixel.
   def _get_neighbors(self, height, width, pixel):
      return np.mgrid[
         max(0, pixel[0] - 1):min(height, pixel[0] + 2),
         max(0, pixel[1] - 1):min(width, pixel[1] + 2)
      ].reshape(2, -1).T

   def apply(self, image):
      current_label = 0
      flag = False
      fifo = deque()

      height, width = image.shape
      total = height * width
      labels = np.full((height, width), self.INIT, np.int32)

      reshaped_image = image.reshape(total)
      # [y, x] pairs of pixel coordinates of the flattened image.
      pixels = np.mgrid[0:height, 0:width].reshape(2, -1).T
      # Coordinates of neighbour pixels for each pixel.
      neighbours = np.array([self._get_neighbors(height, width, p) for p in pixels])
      if len(neighbours.shape) == 3:
         # Case where all pixels have the same number of neighbours.
         neighbours = neighbours.reshape(height, width, -1, 2)
      else:
         # Case where pixels may have a different number of pixels.
         neighbours = neighbours.reshape(height, width)

      indices = np.argsort(reshaped_image)
      sorted_image = reshaped_image[indices]
      sorted_pixels = pixels[indices]

      # self.levels evenly spaced steps from minimum to maximum.
      levels = np.linspace(sorted_image[0], sorted_image[-1], self.levels)
      level_indices = []
      current_level = 0

      # Get the indices that deleimit pixels with different values.
      for i in range(total):
         if sorted_image[i] > levels[current_level]:
            # Skip levels until the next highest one is reached.
            while sorted_image[i] > levels[current_level]: current_level += 1
            level_indices.append(i)
      level_indices.append(total)

      start_index = 0
      for stop_index in level_indices:
         # Mask all pixels at the current level.
         for p in sorted_pixels[start_index:stop_index]:
            labels[p[0], p[1]] = self.MASK
            # Initialize queue with neighbours of existing basins at the current level.
            for q in neighbours[p[0], p[1]]:
               # p == q is ignored here because labels[p] < WSHD
               if labels[q[0], q[1]] >= self.WSHD:
                  labels[p[0], p[1]] = self.INQE
                  fifo.append(p)
                  break

         # Extend basins.
         while fifo:
            p = fifo.popleft()
            # Label p by inspecting neighbours.
            for q in neighbours[p[0], p[1]]:
               # Don't set lab_p in the outer loop because it may change.
               lab_p = labels[p[0], p[1]]
               lab_q = labels[q[0], q[1]]
               if lab_q > 0:
                  if lab_p == self.INQE or (lab_p == self.WSHD and flag):
                     labels[p[0], p[1]] = lab_q
                  elif lab_p > 0 and lab_p != lab_q:
                     labels[p[0], p[1]] = self.WSHD
                     flag = False
               elif lab_q == self.WSHD:
                  if lab_p == self.INQE:
                     labels[p[0], p[1]] = self.WSHD
                     flag = True
               elif lab_q == self.MASK:
                  labels[q[0], q[1]] = self.INQE
                  fifo.append(q)

         # Detect and process new minima at the current level.
         for p in sorted_pixels[start_index:stop_index]:
            # p is inside a new minimum. Create a new label.
            if labels[p[0], p[1]] == self.MASK:
               current_label += 1
               fifo.append(p)
               labels[p[0], p[1]] = current_label
               while fifo:
                  q = fifo.popleft()
                  for r in neighbours[q[0], q[1]]:
                     if labels[r[0], r[1]] == self.MASK:
                        fifo.append(r)
                        labels[r[0], r[1]] = current_label

         start_index = stop_index

      return labels
```

##### Numpy

`np.full(shape, fill_value, dtype=None, order='C')`使用`fill_value`填充`shape`返回`array`

`np.mgrid()`

##### fread()

注意如果是定义了指针，一定要`malloc`分配内存空间，否则读取文件时会出错

##### 初始化结构体

### 参考资料

##### 环境配置

[windows 10上使用vscode编译运行和调试C/C++](https://zhuanlan.zhihu.com/p/77645306)

[VSCode 添加右键菜单 Open With Code(通过 Code 打开)](https://www.cnblogs.com/TopStop/p/15050793.html)

[windows下安装mingw-w64](https://www.cnblogs.com/TAMING/p/9945389.html)

##### C语言

[头文件引用](https://www.runoob.com/cprogramming/c-header-files.html)

[C使用FILE指针文件操作](https://www.cnblogs.com/whiteyun/archive/2009/08/08/1541822.html)

##### 图片格式

[bmp图片灰度化与二值化](https://zhuanlan.zhihu.com/p/122960643)

##### Python实现分水岭算法

[Python的 numpy中 meshgrid 和 mgrid 的区别和使用](https://www.cnblogs.com/shenxiaolin/p/8854197.html)

##### 高斯滤波

[图像处理基础(4)：高斯滤波器详解](https://www.cnblogs.com/wangguchangqing/p/6407717.html)

