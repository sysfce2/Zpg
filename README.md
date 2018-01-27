# libZpg
Library & Tools for Game Assets Packaging

## Dependencies
- zlib

## Installation
```sh
~$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE:STRING=Release . && make && sudo make install
```

## Basic Usage Example
#### Write
```cpp
#include <Zpg/Zpg.hpp>

int main()
{
  Zpg myZpg;
  
  if (!myZpg.addFromFile("assets/images/image.png", "data/image.png") 
        || !myZpg.addFromFile("assets/docs/text.txt", "data/mytext.txt"))
    return -1;
    
  myZpg.saveToFile("myassets.zpg");

  return 0;
}
```

#### Read
```cpp
#include <Zpg/Zpg.hpp>

int main()
{
  Zpg myZpg;
  if (!myZpg.load("myassets.zpg"))
    return -1;
    
  unsigned long imageSize = 0;
  const unsigned char *pFileData = myZpg.getFileData("data/image.png", &imageSize);
  // Do something with 'pFileData'
  unsigned long textSize = 0;
  const unsigned char *pFileTextData = myZpg.getFileData("data/mytext.txt", &textSize);
  std::string myString = Zpg::toString(pFileTextData, textSize);
  // Do something with 'myString'
  
  // Don't need free any data... LibZpg handles it by itself.
  // But you can 'force' it: myZpg.unloadAll()
  return 0;
}
```
