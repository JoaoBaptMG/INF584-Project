#pragma once

namespace util
{
    // Just an utility power function, that is constexpr since pow is not
    constexpr float staticPow(float v, int k)
    {
        if (k < 0) return staticPow(1 / v, -k);
        else if (k == 0) return 1.0;
        else
        {
            double vk2 = staticPow(v, k / 2);
            if (k % 2 == 1) return vk2 * vk2 * v;
            else return vk2 * vk2;
        }
    }
}