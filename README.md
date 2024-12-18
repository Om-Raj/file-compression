# file-compression

An optimized file compression tool using Canonical Huffman Encoding.
It features two utilities :-
- Compressor (`comz`)
- Decompressor (`domz`)

# comz
A simple algorithm to encode data into optimal binary code. It outputs a compressed `.omz` file in the same directory as the original file.

It allows an optional flag `-v` or `-V` for verbose logs.
### Build
```
$ g++ comz.cpp -o comz
```
### Usage
```
$ g++ comz.cpp -o comz
$ ./comz file.txt
$ ./comz file.txt -v
```
### Output
```
file.txt.omz
```

# domz
A simple algorithm to decode compressed data from `.omz` file. It outputs a decompressed `.dec` file to avoid name conflict with the original file (if present in the same directory).

It allows an optional flag `-v` or `-V` for verbose logs. 
### Build
```
$ g++ domz.cpp -o domz
```
### Usage
```
$ ./domz file.txt.omz
$ ./domz file.txt.omz -v
```
### Output
```
file.txt.dec
```

## Why `.omz` file?
While writing the code I didn't want to write long name generic name such as compressor / decompressor. So i went with these short names which actually feature my name **Om**. ;-)