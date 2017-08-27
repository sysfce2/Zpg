# LibZpg
Library & Tools for Game Assets Packaging

## Dependencies
- zlib

## Basic Usage Example
#### Write
```cpp
#include "LibZpg.hpp"

int main()
{
  LibZpg myZpg;
  
  if (!myZpg.addFromFile("assets/images/image.png", "data/image.png") 
        || !myZpg.addFromFile("assets/docs/text.txt", "data/mytext.txt")))
    return -1;
    
  myZpg.saveToFile("myassets.zpg");

  return 0;
}
```

#### Read
```cpp
#include "LibZpg.hpp"

int main()
{
  LibZpg myZpg;
  if (!myZpg.load("myassets.zpg"))
    return -1;
    
  unsigned long imageSize = 0;
  const unsigned char *pFileData = myZpg.getFileData("data/image.png", &fileSize);
  unsigned long textSize = 0;
  const unsigned char *pFileTextData = myZpg.getFileData("data/mytext.txt", &textSize, false);
  
  return 0; // Don't need free any data... LibZpg handles it by itself. (But you can 'force' it: myZpg.unloadAll())
}
```
