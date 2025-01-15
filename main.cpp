#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <fstream>
#include <mutex>

std::mutex mtx;

const uint8_t flatIndexes[] = {
    0, 2, 8, 10, 0, 2, 12, 14, 0, 3, 8, 11, 0, 3, 12, 15,
    1, 2, 9, 10, 1, 2, 13, 14, 1, 3, 9, 11, 1, 3, 13, 15,
    4, 6, 8, 10, 4, 7, 8, 11, 5, 6, 9, 10, 5, 6, 13, 14,
    4, 7, 12, 15, 4, 6, 12, 14, 5, 7, 9, 11, 5, 7, 13, 15,
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};


inline bool checkGroup(const std::vector<uint8_t>& v, const uint8_t* group)
{
    uint8_t counts[4] = {0}; // Frequency counts for 0, 1, 2, 3
    for (int i = 0; i < 4; ++i)
    {
        counts[v[group[i]]]++;
        if (counts[v[group[i]]] == 3)
        {
            // Early exit if 3 matches
            return true;
        }
    }
    return false;
}


bool checkEmbeddedOr(const std::vector<uint8_t>& v)
{
    const size_t numGroups = sizeof(flatIndexes) / 4 / sizeof(uint8_t);
    for (size_t i = 0; i < numGroups; ++i)
    {
        if (checkGroup(v, &flatIndexes[i * 4]))
        {
            return true;
        }
    }
    return false;
}

inline uint8_t posMod(int i)
{
    return (i % 4 + 4) % 4;
}

bool checkPermittedSum(const std::vector<uint8_t>& v)
{
    if (posMod(v[0] + v[5]) % 4 != posMod(v[1] + v[4]))
    {
        return false;
    }

    if (posMod(v[0] - v[5]) % 4 != posMod(v[1] - v[4]))
    {
        return false;
    }

    if (posMod(v[2] + v[3]) % 4 != posMod(v[6] + v[7]))
    {
        return false;
    }

    if (posMod(v[2] - v[3]) % 4 != posMod(v[6] - v[7]))
    {
        return false;
    }

    if (posMod(v[8] + v[13]) % 4 != posMod(v[12] + v[9]))
    {
        return false;
    }

    if (posMod(v[8] - v[13]) % 4 != posMod(v[12] - v[9]))
    {
        return false;
    }

    if (posMod(v[10] + v[15]) % 4 != posMod(v[14] + v[11]))
    {
        return false;
    }

    if (posMod(v[10] - v[15]) % 4 != posMod(v[14] - v[11]))
    {
        return false;
    }

    return true;
}

bool collapsable(const std::vector<uint8_t>& v)
{
    auto temp = v;
    for (auto& e : temp)
    {
        if (e != 0)
        {
            e = 1;
        }
    }

    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 1)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 2)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 3)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 1 || e == 0)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 2 || e == 0)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    temp = v;
    for (auto& e : temp)
    {
        if (e == 3 || e == 0)
        {
            e = 0;
        }
        else
        {
            e = 1;
        }
    }
    if (!checkEmbeddedOr(temp))
    {
        return false;
    }

    return true;

}

int main()
{
    const int n = 16;
    const int base = 4;
    const size_t totalCombinations = static_cast<size_t>(std::pow(base, n));

    std::atomic<size_t> embeddedCount = 0;
    std::atomic<size_t> permittedCount = 0;
    std::atomic<size_t> collapsableCount = 0;

    std::atomic<size_t> count = 0;
    std::vector<std::vector<uint8_t>> result;

    std::vector<std::vector<uint8_t>> embeddedOrSamples;
    std::vector<std::vector<uint8_t>> permittedSamples;

    // Lambda to process a range of combinations in parallel
    auto processRange = [&](size_t start, size_t end)
    {
        for (size_t i = start; i < end; ++i)
        {
            std::vector<uint8_t> v(n);
            size_t num = i;

            for (int j = 0; j < n; ++j)
            {
                v[j] = num % base;
                num /= base;
            }

            if (!checkEmbeddedOr(v))
            {
                embeddedCount++;
                if (embeddedCount % 1000000 == 0)
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    embeddedOrSamples.push_back(v);
                }
                if (checkPermittedSum(v))
                {
                    permittedCount++;
                    if (!collapsable(v))
                    {
                        collapsableCount++;
                        std::lock_guard<std::mutex> lock(mtx);
                        result.push_back(v);
                    }
                    else
                    {
                        if (permittedCount % 1000 == 0)
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            permittedSamples.push_back(v);
                        }
                    }

                }
            }

            if (++count % 100000000 == 0)
            {
                std::cout << "Processed: " << count << " combinations.\n";
                std::cout << "Valid results: " << result.size() << "\n";
                std::cout << "Embedded: " << embeddedCount << "\n";
                std::cout << "Permitted: " << permittedCount << "\n";
                std::cout << "Collapsable: " << collapsableCount << "\n";

            }
        }
    };

    // Number of threads
    const size_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    // Divide work among threads
    size_t chunkSize = totalCombinations / numThreads;
    for (size_t t = 0; t < numThreads; ++t)
    {
        size_t start = t * chunkSize;
        size_t end = (t == numThreads - 1) ? totalCombinations : start + chunkSize;
        threads.emplace_back(processRange, start, end);
    }

    // Wait for threads to finish
    for (auto& thread : threads)
    {
        thread.join();
    }

    std::cout << "Total processed: " << count << "\n";
    std::cout << "Total valid results: " << result.size() << "\n";
    std::cout << "Embedded: " << embeddedCount << "\n";
    std::cout << "Permitted: " << permittedCount << "\n";
    std::cout << "Collapsable: " << collapsableCount << "\n";

    // Write the result to csv
    std::cout << "Writing to file...\n";
    std::ofstream file("result.csv");
    for (const auto& v : result)
    {
        for (size_t i = 0; i < n; ++i)
        {
            file << static_cast<int>(v[i]);
            if (i < n - 1)
            {
                file << ",";
            }
        }
        file << "\n";
    }

    std::ofstream embeddedFile("embedded.csv");
    for (const auto& v : embeddedOrSamples)
    {
        for (size_t i = 0; i < n; ++i)
        {
            embeddedFile << static_cast<int>(v[i]);
            if (i < n - 1)
            {
                embeddedFile << ",";
            }
        }
        embeddedFile << "\n";
    }

    std::ofstream permittedFile("permitted.csv");
    for (const auto& v : permittedSamples)
    {
        for (size_t i = 0; i < n; ++i)
        {
            permittedFile << static_cast<int>(v[i]);
            if (i < n - 1)
            {
                permittedFile << ",";
            }
        }
        permittedFile << "\n";
    }
    return 0;
}
