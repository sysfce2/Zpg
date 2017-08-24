#include <cstdio>
#include <cassert>
#include <LibZpg.hpp>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    LibZpg myZ;

    if(argc>1)
    {
        for(int i=1; i<argc; i++)
            myZ.add(argv[i]);
        myZ.toFile("pack.xzp");
    }

    myZ.read("pack.xzp");

    unsigned char *buffer;
    if(myZ.load("pack.xzp", "img.png", &buffer)) // loads test.txt from pack.xzp into buffer
    {
        cout << "Buffer: " << buffer << endl;

        // ofstream out("ok.png", ios::binary);
        // out << buffer;
    }

    delete buffer;
}
