# myreedsolomon

* 《大数据存储技术》课程作业。
* 一个简单的Reed-Solomon纠删码的编码与解码实验。
* 依赖于[reedsolomon](https://pkg.go.dev/github.com/klauspost/reedsolomon@v1.9.12)库。
* 源码参考[博客](https://cloud.tencent.com/developer/article/1829995)和[klauspost/reedsolomon](https://github.com/klauspost/reedsolomon)。

## 项目文件结构
```
(base) jieyr@robotics191:~/code/myreedsolomon$ tree
.
├── file                   
│   └── example_file       # 源文件
├── go.mod
├── go.sum
├── main.go
├── README.md
├── recover                
│   └── example_file       # RS解码/恢复后的文件
├── shards                 # 存放shards的文件夹
│   ├── example_file.0
|   |   ...
│   └── example_file.5
└── src
    ├── simple-decoder.go  # 简单解码器
    ├── simple-encoder.go  # 简单编码器
    ├── stream-decoder.go  # 优化后的解码器
    └── stream-encoder.go  # 优化后的编码器

4 directories, 16 files
```

## 运行过程
### 编码
编码器读取`./file/example_file`，进行切分和编码操作，把结果存入`./shards/`文件夹中。编码器默认的源数据分割`n=4`新增校验`m=2`。最终得到6个`shards`。
```
$ cd HERE
$ go run src/stream-encoder.go --in=./file/example_file
Opening ./file/example_file

Create the resulting files ...
Creating example_file.0
Creating example_file.1
Creating example_file.2
Creating example_file.3
Creating example_file.4
Creating example_file.5

File split into 4 data + 2 parity shards.
```

### 随机删除
模拟存储过程的随机性，随机删除一些`shards`。
```
$ rm shards/example_file.1 shards/example_file.4
$ ls shards/
example_file.0  example_file.2  example_file.3  example_file.5
```

### 解码
解码器从`./shards/`文件夹读取所有`shards`。解码得到恢复文件并存到`./recover/`文件夹。解码器的参数设置与编码器相同。
```
$ go run src/stream-decoder.go --in=./shards/example_file

Try to open shards ...
Opening ./shards/example_file.0
Opening ./shards/example_file.1
Error reading file open ./shards/example_file.1: no such file or directory
Opening ./shards/example_file.2
Opening ./shards/example_file.3
Opening ./shards/example_file.4
Error reading file open ./shards/example_file.4: no such file or directory
Opening ./shards/example_file.5

Verification failed. Reconstructing data ...
Opening ./shards/example_file.0
Opening ./shards/example_file.1
Error reading file open ./shards/example_file.1: no such file or directory
Opening ./shards/example_file.2
Opening ./shards/example_file.3
Opening ./shards/example_file.4
Error reading file open ./shards/example_file.4: no such file or directory
Opening ./shards/example_file.5
Creating ./shards/example_file.1
Creating ./shards/example_file.4
Reconstruct finished!

Verifiing shards ...
Opening ./shards/example_file.0
Opening ./shards/example_file.1
Opening ./shards/example_file.2
Opening ./shards/example_file.3
Opening ./shards/example_file.4
Opening ./shards/example_file.5
Verification finished!

Writing data to recover/example_file
Opening ./shards/example_file.0
Opening ./shards/example_file.1
Opening ./shards/example_file.2
Opening ./shards/example_file.3
Opening ./shards/example_file.4
Opening ./shards/example_file.5
```

### 比较源文件与解码后的文件
在Linux下采用`hexdump -C FILE`命令输出文件的十六进制。
```
$ hexdump -C file/example_file
00000000  41 62 73 74 72 61 63 74  0a 57 65 20 70 72 6f 70  |Abstract.We prop|
00000010  6f 73 65 20 61 20 66 72  61 6d 65 77 6f 72 6b 20  |ose a framework |
00000020  66 6f 72 20 74 69 67 68  74 6c 79 2d 63 6f 75 70  |for tightly-coup|
00000030  6c 65 64 20 6c 69 64 61  72 20 69 6e 65 72 74 69  |led lidar inerti|
00000040  61 6c 20 6f 64 6f 6d 65  74 72 79 20 76 69 61 20  |al odometry via |
00000050  73 6d 6f 6f 74 68 69 6e  67 20 61 6e 64 20 6d 61  |smoothing and ma|
00000060  70 70 69 6e 67 20 4c 49  4f 2d 53 41 4d 2c 20 74  |pping LIO-SAM, t|
00000070  68 61 74 20 61 63 68 69  65 76 65 73 20 68 69 67  |hat achieves hig|
00000080  68 6c 79 20 61 63 63 75  72 61 74 65 2c 20 72 65  |hly accurate, re|
00000090  61 6c 2d 74 69 6d 65 20  6d 6f 62 69 6c 65 20 72  |al-time mobile r|
000000a0  6f 62 6f 74 20 74 72 61  6a 65 63 74 6f 72 79 20  |obot trajectory |
000000b0  65 73 74 69 6d 61 74 69  6f 6e 20 61 6e 64 20 6d  |estimation and m|
000000c0  61 70 2d 62 75 69 6c 64  69 6e 67 2e 0a           |ap-building..|
000000cd
$ hexdump -C recover/example_file
00000000  41 62 73 74 72 61 63 74  0a 57 65 20 70 72 6f 70  |Abstract.We prop|
00000010  6f 73 65 20 61 20 66 72  61 6d 65 77 6f 72 6b 20  |ose a framework |
00000020  66 6f 72 20 74 69 67 68  74 6c 79 2d 63 6f 75 70  |for tightly-coup|
00000030  6c 65 64 20 6c 69 64 61  72 20 69 6e 65 72 74 69  |led lidar inerti|
00000040  61 6c 20 6f 64 6f 6d 65  74 72 79 20 76 69 61 20  |al odometry via |
00000050  73 6d 6f 6f 74 68 69 6e  67 20 61 6e 64 20 6d 61  |smoothing and ma|
00000060  70 70 69 6e 67 20 4c 49  4f 2d 53 41 4d 2c 20 74  |pping LIO-SAM, t|
00000070  68 61 74 20 61 63 68 69  65 76 65 73 20 68 69 67  |hat achieves hig|
00000080  68 6c 79 20 61 63 63 75  72 61 74 65 2c 20 72 65  |hly accurate, re|
00000090  61 6c 2d 74 69 6d 65 20  6d 6f 62 69 6c 65 20 72  |al-time mobile r|
000000a0  6f 62 6f 74 20 74 72 61  6a 65 63 74 6f 72 79 20  |obot trajectory |
000000b0  65 73 74 69 6d 61 74 69  6f 6e 20 61 6e 64 20 6d  |estimation and m|
000000c0  61 70 2d 62 75 69 6c 64  69 6e 67 2e 0a 00 00 00  |ap-building.....|
000000d0
```