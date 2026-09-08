#include <print>
#include <unistd.h>

int main()
{
    int x = 100;
    int rc = fork();
    if (rc < 0)
    {
        std::println("fork failed");
        return 0;
    }
    else if (rc == 0)
    {
        std::println("child {} x = {}", getpid(), x);
    }
    else
    {
        std::println("parent {} x = {}", getpid(), x);
    }

    return 0;
}