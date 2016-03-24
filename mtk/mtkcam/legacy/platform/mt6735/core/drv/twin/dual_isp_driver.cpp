#include "MyUtils.h"

/* define to replace macro */
#define print_error_message(...) dual_isp_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_isp_get_current_file_name(__VA_ARGS__)
/* func prototype */
static const char *dual_isp_print_error_message(ISP_MESSAGE_ENUM n);
static const char *dual_isp_get_current_file_name(const char *filename);
static ISP_MESSAGE_ENUM dual_cmp_reg(const isp_reg_t *ptr_isp_reg,
                const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param,
                const ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param, const char *dir_path);
static ISP_MESSAGE_ENUM dual_printf_in_config(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, const char *dir_path);
/* extern */
extern ISP_MESSAGE_ENUM dual_cal_main(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param,
                                      ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);

/* return 0: pass, 1: error */
int dual_cal_platform(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
                ISP_DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config,
                const isp_reg_t *ptr_isp_reg, const char *dir_path)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (NULL == dir_path)
    {
        /* platform to reset output */
        memset(ptr_dual_out_config, 0x0, sizeof(ISP_DUAL_OUT_CONFIG_STRUCT));
    }
    result = dual_cal_main(ptr_dual_in_config, ptr_dual_out_config);
    if (ISP_MESSAGE_OK == result)
    {
        result = dual_cmp_reg(ptr_isp_reg, ptr_dual_in_config, ptr_dual_out_config, dir_path);
    }
    if ((ISP_MESSAGE_OK != result) || (ptr_dual_in_config->DEBUG.DUAL_LOG_EN))
    {
        ISP_MESSAGE_ENUM result_temp = dual_printf_in_config(ptr_dual_in_config, dir_path);
        if (ISP_MESSAGE_OK == result)
        {
            result = result_temp;
        }
    }
    if (ISP_MESSAGE_OK == result)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/* return 0: pass, 1: error */
int dual_print_platform_config(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config)
{
    if (ISP_MESSAGE_OK == dual_printf_in_config(ptr_dual_in_config, NULL))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


static ISP_MESSAGE_ENUM dual_cmp_reg(const isp_reg_t *ptr_isp_reg,
                        const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param,
                        const ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param,
                        const char *dir_path)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_isp_reg && ptr_in_param && ptr_out_param)
    {
#ifdef DUAL_PLATFORM_DRIVER
        uart_printf("%s %s %s\n", DUAL_CMP_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
        DUAL_HW_LUT(DUAL_CMP_PRINT_TXT_UART, uart_printf, ptr_isp_reg, ptr_in_param, ptr_out_param, result,);
        uart_printf("%s %s\n", DUAL_CMP_DUMP_HEADER, DUAL_DUMP_END);
#else
        if (dir_path)
        {
            FILE *fpt_log=NULL;
            char full_name[MAX_COMMAND_LENGTH];
            mtk_sprintf(full_name, sizeof(full_name), "%s%s%s", dir_path, FOLDER_SYMBOL_STR, DUAL_ISP_REG_CMP_FILENAME);
            mtk_fopen(fpt_log, full_name, "wb");
            if (NULL == fpt_log)
            {
                result = ISP_MESSAGE_FILE_OPEN_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            printf("\nOutput file: %s\n", full_name);
            fprintf(fpt_log, "%s %s %s\n", DUAL_CMP_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
            DUAL_HW_LUT(DUAL_CMP_PRINT_TXT, fpt_log, fprintf, ptr_isp_reg, ptr_in_param, ptr_out_param, result);
            fprintf(fpt_log, "%s %s\n", DUAL_CMP_DUMP_HEADER, DUAL_DUMP_END);
            fclose(fpt_log);
        }
        else
        {
            return result;
        }
#endif
        /* check error */
        if (ISP_MESSAGE_OK == result)
        {
            dual_driver_printf("HW reg comp pass\n");
        }
        else
        {
            dual_driver_printf("Error: %s\n", print_error_message(result));
        }
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_printf_in_config(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, const char *dir_path)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param)
    {
#ifdef DUAL_PLATFORM_DRIVER
        uart_printf("%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
        DUAL_DEBUG_LUT(DUAL_LOG_PRINT_TXT_UART, uart_printf, ptr_in_param,,,,);
        DUAL_SW_LUT(DUAL_LOG_PRINT_TXT_UART, uart_printf, ptr_in_param,,,,);
        DUAL_HW_LUT(DUAL_LOG_PRINT_TXT_UART_HW, uart_printf, ptr_in_param,,,,);
        uart_printf("%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
#else
        if (dir_path)
        {
            FILE *fpt_log=NULL;
            char full_name[MAX_COMMAND_LENGTH];
            mtk_sprintf(full_name, sizeof(full_name), "%s%s%s", dir_path, FOLDER_SYMBOL_STR, DUAL_IN_CONFIG_FILENAME);
            mtk_fopen(fpt_log, full_name, "wb");
            if (NULL == fpt_log)
            {
                result = ISP_MESSAGE_FILE_OPEN_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            printf("\nOutput file: %s\n", full_name);
            fprintf(fpt_log, "%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
            DUAL_DEBUG_LUT(DUAL_LOG_PRINT_TXT, fpt_log, fprintf, ptr_in_param,,,);
            DUAL_SW_LUT(DUAL_LOG_PRINT_TXT, fpt_log, fprintf, ptr_in_param,,,);
            DUAL_HW_LUT(DUAL_LOG_PRINT_TXT_HW, fpt_log, fprintf, ptr_in_param,,,);
            fprintf(fpt_log, "%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
            fclose(fpt_log);
        }
        else
        {
            return result;
        }
#endif
    }
    return result;
}

static const char *dual_isp_print_error_message(ISP_MESSAGE_ENUM n)
{
    GET_ERROR_NAME(n);
}

static const char *dual_isp_get_current_file_name(const char *filename)
{
    char *ptr = strrchr((char *)filename, FOLDER_SYMBOL_CHAR);
    if (NULL == ptr)
    {
        return filename;
    }
    else
    {
        return (const char *)(ptr + 1);
    }
}
