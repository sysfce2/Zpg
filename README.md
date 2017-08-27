# LibZpg
Library & Tools for Game Assets Packaging

### Dependencies
- zlib

### Basic Usage Example
```cpp
#include "LibZpg.hpp"

int main()
{
  LibZpg myZpg;
  if (!myZpg.load("myassests.zpg"))
    return -1;
  unsigned long imageSize = 0;
  const unsigned char *pFileData = myZpg.getFileData("data/image.png", &fileSize);
  unsigned long textSize = 0;
  const unsigned char *pFileTextData = myZpg.getFileData("data/mytext.txt", &textSize, false);
  return 0; // Don't need free any data... LibZpg handles it by itself. (But you can 'force' it: myZpg.unloadAll())
}
```
