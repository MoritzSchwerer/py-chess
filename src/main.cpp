#include <iostream>

#include "game_env.hpp"

std::vector<int> getActionsFromMask(const std::vector<bool>& mask) {
    std::vector<int> result;
    for (int i = 0; i < mask.size(); ++i) {
        if (mask[i]) {
            result.push_back(i);
        }
    }
    return result;
}

std::string getActionInfo(const int action, bool isWhite) {
    const char files[] = "abcdefgh";
    std::string result = "";
    const uint16_t sourceFile = action / (NUM_ACTION_PLANES * 8);
    const uint16_t sourceRank = (action / NUM_ACTION_PLANES) % 8;
    // const uint16_t tempSourceSquare = sourceRank * 8 + sourceFile;
    const uint16_t plane = action % NUM_ACTION_PLANES;

    uint16_t sourceSquare =
        convertToColorSquare<true>(sourceFile + sourceRank * 8);
    int16_t offset = Lookup::getOffsetFromPlane<true>(plane);
    if (!isWhite) {
        offset = Lookup::getOffsetFromPlane<false>(plane);
        sourceSquare = convertToColorSquare<false>(sourceFile + sourceRank * 8);
    }
    uint16_t sf = sourceSquare % 8;
    uint16_t sr = sourceSquare / 8;

    result += files[sf] + std::to_string(sr) + " " + std::to_string(offset);
    return result;
}

std::string actionVectorToString(const std::vector<int>& vect, bool isWhite) {
    std::string result = "[";
    for (size_t i = 0; i < vect.size(); ++i) {
        // result += std::to_string(vect[i]) + " (" + getActionInfo(vect[i],
        // isWhite) + ")";
        result += "(" + getActionInfo(vect[i], isWhite) + ")";
        if (i != vect.size() - 1) {
            result += ", ";
        }
    }
    result += "] (" + std::to_string(vect.size()) + ")";
    return result;
}

int main() {
    const std::vector<int> actionsTaken = {
        645,  2421, 6,    2340, 4173, 2416, 2413, 77,   1373, 661,  1829, 1170,
        2340, 672,  3563, 4173, 4089, 1986, 2412, 1245, 3589, 4,    1253, 2930,
        938,  1759, 1756, 2557, 2922, 2409, 3727, 3068, 2337, 3290, 2486, 3804,
        3524, 1900, 1835, 1392, 804,  2497, 222,  87,   1391, 3806, 1754, 1758,
        1191, 2562, 3800, 2337, 1244, 1242, 150,  1898, 234,  737,  2997, 658,
        1542, 3212, 384,  1767, 1175, 3066, 3070, 2562, 585,  1829, 14,   2421,
        1826, 1902, 152,  3216, 1169, 1246, 745,  3563, 2048, 3288, 1247, 95,
        2134, 3000, 1830, 643,  585,  1831, 2340, 3219, 54,   3124, 669,  1754,
        4113, 4092, 1774, 3876, 3532, 2556, 2415, 1251, 2559, 2393, 3800, 1987,
        805,  1302, 2996, 2027, 3873, 4164, 3945, 4105, 3896, 4530, 3605, 2353,
        3872, 588,  3797, 4602, 2921, 2415, 2342, 792,  77,   685,  3215, 2997,
        2921, 3069, 2340, 38,   3142, 2999, 2410, 206,  1825, 2932, 3086, 976,
        3299, 693,  150,  1886, 4478, 2617, 2123, 1052, 2728, 3359, 1169, 3711,
        296,  2777, 369,  2705, 223,  3602, 506,  2778, 588,  1279, 661,  2856,
        564,  1709, 4090, 4165, 3595, 4251, 2054, 2486, 731,  4058, 1043, 2561,
        2562, 3597, 150,  645,  296,  4438, 3230, 4529, 1700, 3711, 369,  1371,
        3219, 281,  506,  4439, 3877, 3802, 224,  1377, 731,  3998, 2568, 4454,
        152,  4381, 730,  2613, 3595, 1881, 2065, 2762, 532,  645,  2046, 3823,
        3584, 3869, 1538, 3491, 4268, 4311, 2046, 3928, 1692, 3510, 2709, 3217,
        1418, 3489, 1541, 3727, 250,  1391, 2855, 3803, 1540, 2763, 1900, 4089,
        2192, 3516, 1692, 3656, 1391, 4245, 2703, 4129, 2195, 585,  79,   52,
        661,  4454, 734,  522,  2703, 392,  2195, 87,   2714, 3871, 806,  1271,
        3730, 3613, 734,  3953, 1464, 3805, 803,  3364, 149,  3947, 1537, 2642,
        4390, 4454, 77,   3876, 149,  3808, 78,   3953, 3365, 3816, 588,  4058,
        658,  3597, 1610, 3808, 76,   3951, 4039, 2783, 1700, 4527, 3237, 3945,
        906,  3876, 3222, 4527, 2196, 3946, 1389, 1649, 2799, 4018, 905,  3948,
        2954, 1198, 476,  3508, 3401, 3605, 2972, 3904, 378,  3556, 4,    4048,
        3466, 1130, 3068, 4384, 1408, 776,  2559, 4528, 2647, 4251, 2432, 2491,
        2954, 1317, 378,  1250, 80,   95,   4382, 4020, 1397, 4530, 370,  4457,
        737,  1831, 3904, 4590, 1392};

    const std::vector<int> actualActions = {
        2410, 2412, 2413, 2415, 2418, 2421, 2423, 2426, 2429, 2431,
        2434, 2437, 2445, 2453, 4071, 4073, 4075, 4599, 4602};

    ChessGameEnv env;
    bool isWhite = true;
    for (auto& a : actionsTaken) {
        std::cout << "===================" << std::endl;
        std::cout << getActionInfo(a, isWhite) << " " << isWhite << std::endl;
        env.step(a);
        env.showBoard();
        isWhite = !isWhite;
    }

    // ChessGameEnv env(
    //     "rnbqk1nr/3pp2p/pp6/6p1/1Np1Pp2/BPb4N/P2PBPPP/2KR3R w kq - "
    //     "0 14");
    // auto obs = env.observe();
    // env.showBoard();
    // std::vector<bool> actionMask = obs.actionMask;
    // std::vector<int> actions = getActionsFromMask(actionMask);
    //
    // std::string str = actionVectorToString(actions, true);
    //
    // std::cout << str << std::endl;

    return 0;
}
