#include <iostream>
#include <chrono>
#include <map>

typedef unsigned long integer;

integer f(const integer input)
{
    // this will cache calculated values
    static std::map<integer,integer> memo;
    
    if (input == 0)
        return 1;
    if (input == 1)
        return 1;
    
    // check if input values has been already calculated
    if (memo.count(input) > 0)
        return memo[input];
    
    const integer half = input/2;
    // calculate f(n) when input = 2*n
    const integer s1 = f(half);
    if (input & 1)
    {
        // calculate f(n-1) when input = 2*n+1
        const integer s2 = f(half-1);
        // sum f(n)+f(n-1)
        const integer s = s1+s2;
        // store value in cache
        memo[input] = s;
        return s;
    }
    else
    {
        memo[input] = s1;
        return s1;
    }
    
}


int main(int argc, const char * argv[]) {
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto out = f(123456789012345678);
    std::cout << "f(n)=" << out << std::endl;
    
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    
    return 0;
}
