# DirectoryWatcher

You can find more specific information in DirectoryWatcher/DirectoryWatcher/DirectoryWatcher.h

## Usage Example
```c++
#include <iostream>

#include "DirectoryWatcher.h"

int main(int argc, char* argv[])
{
    // You can use lambda expressions for callbacks
    DirectoryWatcher watcher1("C:\\Program Files\\Git\\tmp\\", [] (string directory, CallbackType type, string details)
    {
        std::cout << "Directory: " << directory << " , change: " << details << std::endl;
    });

    // starts up DirectoryWatcher
    watcher1.Watch(true);
    
    system("pause")
    return 0;
}

```
