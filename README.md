# csv_parser

## https://github.com/JackonYang/csv_parser_RFC4180 的C翻译版

### 运行截图

![运行截图](/home/meanson/code/c/csv_parser/README.assets/运行截图.png)

### 存储

字符串结构体（最大256）

- char数组
- int 数组长度
- int 字符串个数

csv结构体

- 字符串结构体 解码缓存
- char 数组 文件读取缓存
- 字符串结构体数组 csv文件头部
- 字符串结构体数组 解码结果缓存
- 文件指针

注：字符串结构体，添加字符长度和字符串个数便于字符的添加。