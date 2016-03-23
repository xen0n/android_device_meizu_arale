#define LOG_TAG "GuiExt-Test"

#include <string.h>
#include <cutils/log.h>
#include <hardware/gralloc.h>
#include <hardware/hwcomposer_defs.h>

#include "GuiExtClientProducer.h"
#include "GuiExtClientConsumer.h"

#include "gralloc_mtk_defs.h"
#include "graphics_mtk_defs.h"

using namespace android;

GuiExtClientProducer &producer(GuiExtClientProducer::getInstance());
GuiExtClientConsumer &consumer(GuiExtClientConsumer::getInstance());

void print_consumer_info()
{
    const size_t SIZE = 4096;
    char buffer[SIZE];
    String8 result;
    consumer.dump(result, buffer, SIZE);
    printf("%s", result.string());
}

int main(int argc, char *argv[])
{
    int32_t ret = 0;
    int32_t test = 0;
    int32_t arg1 = 0, arg2 = 0, arg3 = 0;
    uint32_t config[HWC_NUM_DISPLAY_TYPES] = {HWC_DISPLAY_PRIMARY, HWC_DISPLAY_EXTERNAL, HWC_DISPLAY_VIRTUAL};

    printf("GuiExt test start, pid=%d\n", getpid());

    if (argc >= 2)
        test = atoi(argv[1]);

    if (argc >= 3)
        arg1 = atoi(argv[2]);

    if (argc >= 4)
        arg2 = atoi(argv[3]);

    if (argc >= 5)
        arg3 = atoi(argv[4]);

    printf("----------------------------------------------\n"
           "[[ test case ]]\n"
           "[ 0] alloc [arg1: alloc number, arg2: config display]\n"
           "[ 1] alloc + free [arg1: re-alloc a new id for arg2, arg2: specific free id if arg1=0]\n"
           "[ 2] alloc + acquire + release [arg1: isHwcNeeded test, arg2: release specific buffer id]\n"
           "[ 3] acquire for GPU buffer [arg1: specific pool id, arg2: acquire number]\n"
           "[ 4] acquire for HWC buffer [arg1: specific pool id, arg2: acquire number, arg3: display type]\n"
           "[ 5] alloc + acquire + release + disconnect\n"
           "[ 6] alloc + acquire + free + disconnect\n"
           "[ 7] alloc + acquire(3) + release(3) + free + acquire(3)\n"
           "[ 8] configDisplay test [arg1: type, arg2: test w/h select, arg3: buffer number]\n"
           "\n"
           "[10] test for alloc(10) + free(5) (arg1: isHwc buffer, arg2: Recursive)\n"
           "[11] stress test for alloc + acquire + free + disconnect (arg1: isHwcNeeded, arg2: test case)\n"
           "[12] stress test for alloc + acquire + free + disconnect (for Multi-Display)\n"
           "----------------------------------------------\n"
        );
    printf("test case[%d] (arg1 = %d, arg2 = %d, arg3 = %d) \n", test, arg1, arg2, arg3);

    switch (test) {
        case 0: {
#define TEST_CASE1_SIZE 3
            uint32_t id = 0;
            uint32_t cc = TEST_CASE1_SIZE;
            uint32_t w[TEST_CASE1_SIZE] = {300, 4096, 720};
            uint32_t h[TEST_CASE1_SIZE] = {800, 2160, 1280};

            if (arg1 > 0)
                cc = arg1;

            arg2 = arg2 % 3;
            if (arg2 > 0)
                consumer.configDisplay(config[arg2], true, 1920, 1080);

            for (uint32_t i = 0; i < cc; i++)
            {
                uint32_t idx = i % TEST_CASE1_SIZE;
                uint32_t gralloc_usage = (i % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
                gralloc_usage |= (i % 3 == 2) ? (uint32_t)GRALLOC_USAGE_SECURE : 0;
                ret = producer.alloc(gralloc_usage, w[idx], h[idx], &id);

                if (ret != NO_ERROR) {
                    printf("alloc pool id[%d]=%d fail, err=%d !!!\n", i, id, ret);
                    break;
                }
                else
                    printf("alloc pool id[%d]=%d OK !!!\n", i, id);
            }
        } break;
        case 1: {
            uint32_t id = 0;
            if (arg1 == 1)
            {
                ret = producer.alloc(GRALLOC_USAGE_HW_COMPOSER, 300, 200, &id);
                printf("alloc pool id=%d\n", id);
                arg2 = id;
                ret = producer.alloc(GRALLOC_USAGE_HW_COMPOSER, 300, 200, &id);
                printf("alloc pool id=%d\n", id);
            }
            ret = producer.free(arg2);
            printf("free id=%d, ret=%d\n", arg2, ret);
        } break;
        case 2: {
            uint32_t id = 0;
            uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
            ret = producer.alloc(gralloc_usage, 300, 200, &id);
            printf("alloc pool id=%d\n", id);
            arg2 = arg2 % 3;

            ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
            for (int i = 0; i < 3; i ++) {
                ret = consumer.acquire(&buf[i], id, arg1);
                if (buf[i] != NULL) {
                    printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                        buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                } else
                    printf("acquire[%d], buf is NULL\n", i);
            }
            ret = consumer.release(buf[arg2], id, arg1);
            printf("release, id=%d, idx=%d, ret=%d\n", id, arg2, ret);
        } break;
        case 3: {
            uint32_t id = arg1;

            if (arg2 > 6)
                arg2 = 6;
            ANativeWindowBuffer *buf[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
            for (int i = 0; i < arg2; i ++) {
                ret = consumer.acquire(&buf[i], id, 0);
                if (buf[i] != NULL) {
                    printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                        buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                } else
                    printf("acquire[%d], buf is NULL\n", i);
            }
        } break;
        case 4: {
            uint32_t id = arg1;

            if (arg2 > 6)
                arg2 = 6;
            arg3 %= HWC_NUM_DISPLAY_TYPES;
            ANativeWindowBuffer *buf[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
            for (int i = 0; i < arg2; i ++) {
                ret = consumer.acquire(&buf[i], id, 1, config[arg3]);
                if (buf[i] != NULL) {
                    printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                        buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                } else
                    printf("acquire[%d], buf is NULL\n", i);
            }
        } break;
        case 5: {
            uint32_t id = 0;
            uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
            ret = producer.alloc(gralloc_usage, 300, 200, &id);
            printf("alloc pool id=%d\n", id);
            arg2 = arg2 % 3;

            ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
            for (int i = 0; i < 3; i ++) {
                ret = consumer.acquire(&buf[i], id, arg1);
                if (buf[i] != NULL) {
                    printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                        buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                } else
                    printf("acquire[%d], buf is NULL\n", i);
            }
            ret = consumer.release(buf[arg2], id, arg1);
            printf("release, id=%d, idx=%d, ret=%d\n", id, arg2, ret);

            print_consumer_info();

            consumer.disconnect(id, arg1);
            printf("disconnect id=%d\n", id);
//            ret = producer.free(id);
//            printf("free id=%d\n", id);
        } break;
        case 6: {
            uint32_t id = 0;
            uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
            ret = producer.alloc(gralloc_usage, 300, 200, &id);
            printf("alloc pool id=%d\n", id);

            ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
            for (int i = 0; i < 3; i ++) {
                ret = consumer.acquire(&buf[i], id, arg1);
                if (buf[i] != NULL) {
                    printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                        buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                } else
                    printf("acquire[%d], buf is NULL\n", i);
            }
            ret = producer.free(id);
            printf("free id=%d\n", id);
            consumer.disconnect(id, arg1);
            printf("disconnect id=%d\n", id);
        } break;
        case 7: {
            uint32_t id = 0;
            uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
            ret = producer.alloc(gralloc_usage, 300, 200, &id);
            printf("alloc pool id=%d\n", id);

            {
                ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
                for (int i = 0; i < 3; i ++) {
                    ret = consumer.acquire(&buf[i], id, arg1);
                    if (buf[i] != NULL) {
                        printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                            buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                    } else
                        printf("acquire[%d], buf is NULL\n", i);
                }
                for (int i = 0; i < 3; i ++) {
                    ret = consumer.release(buf[i], id, arg1);
                    printf("release, id=%d, idx=%d, ret=%d\n", id, i, ret);
                }
                ret = producer.free(id);
                printf("free id=%d\n", id);
            }
            printf("re-acquire buffer\n");
            {
                ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
                for (int i = 0; i < 3; i ++) {
                    ret = consumer.acquire(&buf[i], id, arg1);
                    if (buf[i] != NULL) {
                        printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                            buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                    } else
                        printf("acquire[%d], buf is NULL\n", i);
                }
            }
        } break;
        case 8: {
#define TEST_CASE4_SIZE      4
#define TEST_MAX_BUF_NUM    10
            uint32_t w[TEST_CASE4_SIZE] = {0, 1920, 4096, 720};
            uint32_t h[TEST_CASE4_SIZE] = {0, 1080, 2160, 1280};

            arg1 %= HWC_NUM_DISPLAY_TYPES;
            arg2 %= TEST_CASE4_SIZE;
            arg3 = arg3 < TEST_MAX_BUF_NUM? arg3 : TEST_MAX_BUF_NUM-1;

            uint32_t type = config[arg1];
            printf("config display, type = %d, w=%d, h=%d, numBuf=%d\n", type, w[arg2], h[arg2], arg3);
            if (arg2)
                consumer.configDisplay(type, true, w[arg2], h[arg2], arg3);
            else
                consumer.configDisplay(type, false, w[arg2], h[arg2], arg3);

            if (arg2) {
                uint32_t id;
                ret = producer.alloc(GRALLOC_USAGE_HW_COMPOSER, 300, 200, &id);
                printf("alloc pool id=%d\n", id);

                ANativeWindowBuffer *buf[TEST_MAX_BUF_NUM] = {NULL, NULL, NULL, NULL, NULL,
                                                NULL, NULL, NULL, NULL, NULL};
                for (int i = 0; i < arg3+1; i ++) {
                    ret = consumer.acquire(&buf[i], id, 1, type);
                    if (buf[i] != NULL) {
                        printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                            buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                    } else
                        printf("acquire[%d], buf is NULL\n", i);
                }
                ret = consumer.release(buf[0], id, 1, type);
                printf("release, id=%d, idx=%d, ret=%d\n", id, 0, ret);

                //consumer.disconnect(id, 1, arg2);
            }
        } break;
        // stress test
        case 10: {
            static bool isFirstRun = true;
            uint32_t alloc_num = 10;

            do {
                if (isFirstRun) {
                    alloc_num = 10;
                    isFirstRun = false;
                } else
                    alloc_num = 5;

                uint32_t id[10];
                for (uint32_t i = 0; i < alloc_num; i ++) {
                    uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
                    ret = producer.alloc(gralloc_usage, 300, 200, &id[i]);
                    printf("alloc pool id=%d, ret=%d\n", id[i], ret);
                }
                for (int i = 0; i < 5; i++) {
                    ret = producer.free(id[i*2]);
                    printf("free id=%d\n", id[i*2]);
                }

                if (arg2)
                    break;

                sleep(1);
            } while (true);
        } break;
        case 11: {
#define TEST_CASE11_SIZE 3
            uint32_t w[TEST_CASE11_SIZE] = {1920, 4096, 1280};
            uint32_t h[TEST_CASE11_SIZE] = {1080, 2160, 720};

            arg2 %= TEST_CASE11_SIZE;
            printf("alloc size=[%dx%d]\n", w[arg2], h[arg2]);

            if (arg3) {
                consumer.configDisplay(config[1], true, 1920, 1080);
            }

            do {
                uint32_t id = 0;
                uint32_t gralloc_usage = (arg1 % 2 == 1) ? (uint32_t)GRALLOC_USAGE_HW_COMPOSER : 0;
                ret = producer.alloc(gralloc_usage, w[arg2], h[arg2], &id);
                printf("alloc pool id=%d\n", id);

                ANativeWindowBuffer *buf[3] = {NULL, NULL, NULL};
                for (int i = 0; i < 3; i ++) {
                    ret = consumer.acquire(&buf[i], id, arg1);
                    if (buf[i] != NULL) {
                        printf("acquire[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", i,
                            buf[i]->handle, buf[i]->width, buf[i]->height, buf[i]->stride, buf[i]->format, ret);
                    } else
                        printf("acquire[%d], buf is NULL\n", i);
                }

                sleep(1);

                ret = producer.free(id);
                printf("free id=%d\n", id);
                print_consumer_info();

                consumer.disconnect(id, arg1);
                printf("disconnect id=%d\n", id);

                print_consumer_info();
            } while (true);
        } break;
        case 12: {
            do {
                consumer.configDisplay(config[1], true, 1920, 1080);
                consumer.configDisplay(config[2], true, 1280, 720);

                uint32_t id = 0;
                uint32_t gralloc_usage = (uint32_t)GRALLOC_USAGE_HW_COMPOSER;
                ret = producer.alloc(gralloc_usage, 1280, 720, &id);
                printf("alloc pool id=%d, ret=%d\n", id, ret);

                ANativeWindowBuffer* buf[3][3] = {{NULL, NULL, NULL},
                                                  {NULL, NULL, NULL},
                                                  {NULL, NULL, NULL}};
                for (int j = 0; j < 3; j++) {
                    for (int i = 0; i < 3; i ++) {
                        ret = consumer.acquire(&buf[i][j], id, 1, config[j]);
                        if (buf[i][j] != NULL) {
                            printf("1. acquire, type=%d, idx[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", config[j], i,
                                buf[i][j]->handle, buf[i][j]->width, buf[i][j]->height, buf[i][j]->stride, buf[i][j]->format, ret);
                        } else
                            printf("1. acquire, type=%d, idx[%d], buf is NULL, ret=%d\n", config[j], i, ret);
                    }
                }

                sleep(1);

                ret = producer.free(id);
                printf("free id=%d\n", id);

                for (int j = 0; j < 3; j++) {
                    for (int i = 0; i < 3; i ++) {
                        ret = consumer.release(buf[i][j], id, 1, config[j]);
                        printf("1. release, type=%d, idx[%d], ret=%d\n", i, config[j], ret);
                        ret = consumer.acquire(&buf[i][j], id, 1, config[j]);
                        if (buf[i][j] != NULL) {
                            printf("2. acquire, type=%d, idx[%d], buf=%p, size=[w:%d, h=%d, s=%d, f=%x], ret=%d\n", config[j], i,
                                buf[i][j]->handle, buf[i][j]->width, buf[i][j]->height, buf[i][j]->stride, buf[i][j]->format, ret);
                        } else
                            printf("2. acquire, type=%d, idx[%d], buf is NULL, ret=%d\n", config[j], i, ret);
                    }
                }

                consumer.disconnect(id, 1);
                printf("disconnect id=%d\n", id);
            } while (true);
        }
        default:
            printf("Unsupport test case\n");
    }

    printf("GuiExt test exit, ret=%d\n", ret);
    printf("GuiExt test complete. CTRL+C to finish.\n");
    IPCThreadState::self()->joinThreadPool();

    return(0);
}
