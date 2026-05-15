#include "divider.h"
#include "logger.h"

namespace hw {
    s32 divide(s32 numerator, s32 denominator) {
        if (denominator == 0) {
            logger().error("Dividing by 0!");
            return 0;
        }

        REG_DIVCNT = DIV_32_32;
        REG_DIV_NUMER = (s64)numerator;
        REG_DIV_DENOM = (s64)denominator;

        while (REG_DIVCNT & DIV_BUSY);

        return (s32)REG_DIV_RESULT;
    }

    s32 divide(s32 numerator, s32 denominator, s32& rest) {
        if (denominator == 0) {
            logger().error("Dividing by 0!");
            rest = 0;
            return 0;
        }

        REG_DIVCNT = DIV_32_32;
        REG_DIV_NUMER = (s64)numerator;
        REG_DIV_DENOM = (s64)denominator;

        while (REG_DIVCNT & DIV_BUSY);

        rest = (s32)REG_DIVREM_RESULT;

        return (s32)REG_DIV_RESULT;
    }

    s32 mod(s32 num, s32 den) {
        if (den == 0) {
            logger().error("Dividing by 0!");
            return 0;
        }

        REG_DIVCNT = DIV_32_32;
        REG_DIV_NUMER = (s64)num;
        REG_DIV_DENOM = (s64)den;

        while (REG_DIVCNT & DIV_BUSY);

        return (s32)REG_DIVREM_RESULT;
    }
}