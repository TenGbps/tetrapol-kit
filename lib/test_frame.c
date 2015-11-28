#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "frame.c"

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_diff_dec(void **state)
{
    (void) state;   // unused

    uint8_t fr_data[FRAME_DATA_LEN];
    for (int i = 0; i < FRAME_DATA_LEN; ++i) {
        fr_data[i] = 1 << (i % 7);
    }
    uint8_t data_exp[FRAME_DATA_LEN] = {
        0x01, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18,
        0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06,
        0x0c, 0x14, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41,
        0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30,
        0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c,
        0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21, 0x03,
        0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x50,
        0x41, 0x03, 0x05, 0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14,
        0x30, 0x60, 0x21, 0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05,
        0x0c, 0x18, 0x28, 0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21,
        0x03, 0x06, 0x0a, 0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18, 0x28,
        0x60, 0x41, 0x42, 0x06, 0x0c, 0x14, 0x30, 0x60, 0x21, 0x03, 0x06, 0x0a,
        0x18, 0x30, 0x50, 0x41, 0x03, 0x05, 0x0c, 0x18,
    };

    frame_diff_dec(fr_data);
    assert_memory_equal(data_exp, fr_data, FRAME_DATA_LEN);
}

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_deinterleave(void **state)
{
    (void) state;   // unused

    uint8_t fr_data[FRAME_DATA_LEN];
    for (int i = 0; i < FRAME_DATA_LEN; ++i) {
        fr_data[i] = 0x7f & (i + 1 + 8);
    }

    uint8_t data_exp[FRAME_DATA_LEN] = {
        0x0a, 0x56, 0x2f, 0x7b,
        0x1d, 0x69, 0x44, 0x10, 0x0c, 0x58, 0x32, 0x7e, 0x20, 0x6c, 0x47, 0x13,
        0x0e, 0x5a, 0x35, 0x01, 0x23, 0x6f, 0x4a, 0x16, 0x11, 0x5d, 0x38, 0x04,
        0x26, 0x72, 0x4d, 0x19, 0x14, 0x60, 0x3b, 0x07, 0x29, 0x75, 0x50, 0x1c,
        0x17, 0x63, 0x3e, 0x0a, 0x2c, 0x78, 0x53, 0x1f, 0x1a, 0x66, 0x41, 0x0d,
        0x2e, 0x79, 0x55, 0x1d, 0x0b, 0x61, 0x31, 0x7c, 0x1c, 0x6a, 0x43, 0x0e,
        0x0d, 0x54, 0x34, 0x7f, 0x1f, 0x6d, 0x46, 0x11, 0x10, 0x5e, 0x37, 0x02,
        0x22, 0x70, 0x49, 0x14, 0x13, 0x5b, 0x3a, 0x05, 0x25, 0x73, 0x4c, 0x17,
        0x16, 0x64, 0x3d, 0x08, 0x28, 0x76, 0x52, 0x1a, 0x19, 0x67, 0x40, 0x0b,
        0x2b, 0x7a, 0x4f, 0x20, 0x09, 0x59, 0x30, 0x7d, 0x1e, 0x68, 0x42, 0x0f,
        0x0f, 0x57, 0x33, 0x00, 0x21, 0x6b, 0x45, 0x12, 0x12, 0x5c, 0x36, 0x03,
        0x24, 0x6e, 0x48, 0x15, 0x15, 0x5f, 0x39, 0x06, 0x27, 0x71, 0x4b, 0x18,
        0x18, 0x62, 0x3c, 0x09, 0x2a, 0x74, 0x4e, 0x1b, 0x1b, 0x65, 0x3f, 0x0c,
        0x2d, 0x77, 0x51, 0x1e,
    };

    uint8_t fr_data_deint[FRAME_DATA_LEN];
    frame_deinterleave1(fr_data_deint, fr_data, TETRAPOL_BAND_UHF);
    frame_deinterleave2(fr_data_deint, fr_data, TETRAPOL_BAND_UHF, FRAME_TYPE_DATA);
    assert_memory_equal(data_exp, fr_data_deint, FRAME_DATA_LEN);
}

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_decoder_data_01(void **state)
{
    (void) state;   // unused

    const uint8_t fr_data[FRAME_DATA_LEN] = {
        0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0,
        1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1,
        0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0,
        0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
        1, 1, 0, 1, 1, 1, 0, 0
    };

    const uint8_t data_exp[] = {
        1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1,
        1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
        1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0
    };

    frame_t fr;
    frame_decoder_t *fd = frame_decoder_create(TETRAPOL_BAND_UHF, 67, FRAME_TYPE_DATA);
    assert_non_null(fd);
    frame_decoder_decode(fd, &fr, fr_data);
    assert_int_equal(sizeof(data_exp), sizeof(frame_data_t));
    assert_memory_equal(data_exp, fr.blob_, sizeof(frame_data_t));
    assert_int_equal(0, fr.broken);
    frame_decoder_destroy(fd);
}

// single bit error
static void test_frame_decoder_data_02(void **state)
{
    (void) state;   // unused

    uint8_t fr_data[FRAME_DATA_LEN] = {
        0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0,
        1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1,
        0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0,
        0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
        1, 1, 0, 1, 1, 1, 0, 0
    };

    const uint8_t data_exp[] = {
        1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1,
        1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
        1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0
    };

    frame_decoder_t *fd = frame_decoder_create(TETRAPOL_BAND_UHF, 67, FRAME_TYPE_DATA);
    assert_non_null(fd);

    for (int i = 0; i < FRAME_DATA_LEN; ++i) {
        //printf("Flipped bit no=%d\n", i);
        fr_data[i] ^= 1;
        frame_t fr;
        frame_decoder_decode(fd, &fr, fr_data);
        assert_int_equal(sizeof(data_exp), sizeof(frame_data_t));
        assert_memory_equal(data_exp, fr.blob_, sizeof(frame_data_t));
        assert_int_equal(0, fr.broken);
        fr_data[i] ^= 1;
    }

    frame_decoder_destroy(fd);
}

// the goal is just to make sure the function provides the same results
// after refactorization
static void test_frame_decoder_voice_01(void **state)
{
    (void) state;   // unused

    const uint8_t fr_data[FRAME_DATA_LEN] = {
        0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0,
        1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0,
        0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0,
        0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1,
        0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1,
        1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1,
        0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const uint8_t data_exp[] = {
        0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1,
        1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1,
        1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
        0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0,
        0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1
    };

    frame_t fr;
    frame_decoder_t *fd = frame_decoder_create(TETRAPOL_BAND_UHF, 118, FRAME_TYPE_VOICE);
    assert_non_null(fd);
    frame_decoder_decode(fd, &fr, fr_data);
    assert_int_equal(sizeof(data_exp), sizeof(frame_voice_t));
    assert_memory_equal(data_exp, fr.blob_, sizeof(frame_data_t));
    assert_int_equal(0, fr.broken);
    frame_decoder_destroy(fd);
}

static void test_mk_crc5(void **state)
{
    (void) state;   // unused

    {
        const uint8_t in[] = { 1, 0, 1, 0, 1, 0 };
        const uint8_t out_exp[] = { 0, 1, 0, 1, 1 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }

    {
        const uint8_t in[] = { 0, 0, 0, 0, 0, 0 };
        const uint8_t out_exp[] = { 0, 0, 0, 0, 0 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }

    {
        const uint8_t in[] = { 1, 1, 1, 1, 1, 1 };
        const uint8_t out_exp[] = { 0, 1, 1, 0, 0 };
        uint8_t out[5];
        mk_crc5(out, in, sizeof(out_exp));
        assert_memory_equal(out, out_exp, sizeof(out_exp));
    }
}

// this test expects the correct frame_decode1 implementation
static void test_frame_encode1(void **state)
{
    const uint8_t sol_exp[26] = {
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 1, 0, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 1, 0, 1, 0,
        0, 1,
    };
    uint8_t sol[8];
    memset(sol, 0, sizeof(sol));
    frame_encode1(sol, sol_exp);

    uint8_t bits[52];
    for (int i = 0; i < sizeof(bits); ++i) {
        bits[i] = (sol[i / 8] >> (i % 8)) & 1;
    }

    uint8_t fr_sol[26], fr_errs[26];
    uint8_t fr_errs_exp[26];
    memset(fr_errs_exp, 0, sizeof(fr_errs_exp));
    frame_decode1(fr_sol, fr_errs, bits, FRAME_TYPE_VOICE);
    assert_memory_equal(fr_errs, fr_errs_exp, sizeof(fr_errs_exp));
    assert_memory_equal(fr_sol, sol_exp, sizeof(sol_exp));
}

// this test expects the correct frame_decode2 implementation
static void test_frame_encode2(void **state)
{
    const uint8_t frame_dec[26+50] = {
        1, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 1, 0, 0, 0, 1, 0, 1,
        0, 1, 0, 1, 1, 0, 1, 0,
        0, 0, 1, 0, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 1, 1,
        0, 0, 0, 1, 0, 0, 1, 0,
        0, 0
    };
    uint8_t frame_enc[19];
    memset(frame_enc, 0, sizeof(frame_enc));
    frame_encode2(frame_enc, frame_dec);

    uint8_t bits[152];
    for (int i = 2*26; i < sizeof(bits); ++i) {
        bits[i] = (frame_enc[i / 8] >> (i % 8)) & 1;
    }

    uint8_t frame_dec2[26+50], frame_errs[26+50];
    uint8_t fr_errs_exp[50];
    memset(fr_errs_exp, 0, sizeof(fr_errs_exp));
    frame_decode2(frame_dec2, frame_errs, bits, FRAME_TYPE_DATA);
    assert_memory_equal(fr_errs_exp, frame_errs+26, sizeof(fr_errs_exp));
    assert_memory_equal(frame_dec2+26, frame_dec+26, 50);
}

int main(void)
{
    const UnitTest tests[] = {
        unit_test(test_frame_diff_dec),
        unit_test(test_frame_deinterleave),
        unit_test(test_frame_decoder_data_01),
        unit_test(test_frame_decoder_data_02),
        unit_test(test_frame_decoder_voice_01),
        unit_test(test_mk_crc5),
        unit_test(test_frame_encode1),
        unit_test(test_frame_encode2),
    };

    return run_tests(tests);
}
