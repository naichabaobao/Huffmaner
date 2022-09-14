# Huffmaner
Huffman implementation of c++

#### data
```cpp
    Huffmaner huff;
    auto str = std::string("acc6a6eeeeeac8***++caa");
    char* result;
    int resultSize = 0;
    huff.encodeData((char*)str.c_str(), str.length(), result, resultSize);
    
    char* rawResult;
    int rawResultSize = 0;
    huff.decodeData(result, resultSize, rawResult, rawResultSize);

    std::cout << "source size = " << str.size() << std::endl << "encode size = " << resultSize << std::endl;
    std::cout << "source size = " << resultSize << std::endl << "raw size = " << rawResultSize << std::endl;
    std::cout << result << std::endl << rawResult << std::endl;

```

#### file
```cpp
    Huffmaner huff;
    huff.encodeFile("c:/Users/Administrator/Desktop/source.txt", "c:/Users/Administrator/Desktop/huff.huff");
    huff.decodeFile("c:/Users/Administrator/Desktop/huff.huff", "c:/Users/Administrator/Desktop/result.txt");
```

![loveme](https://user-images.githubusercontent.com/38098031/190124788-043758bf-bdf5-42b7-9bd2-06c9d545d954.png)


