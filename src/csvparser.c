#include <stdio.h>
#include <string.h>

// 最大容量
#define MAX_SIZE 256
// 逗号
#define COMMA 0X2C
// 引号
#define DQUOTE 0X22
// CR
#define CR 0X0D
// LF
#define LF 0X0A

// CSV 解析状态 -- 取决于 ENCLOSURE_CHAR(") 的个数
typedef enum CSV_ENCLOSURE_STATUS_ENUM
{
    CSV_ENCLOSURE_NONE,
    CSV_ENCLOSURE_ENTER,
    CSV_ENCLOSURE_EXIT
} CSV_ENCLOSURE_STATUS_ENUM;

// 字符数组
typedef struct csvString
{
    char str[MAX_SIZE];
    int strLength;
    int stringCount;
} csvString;

// csv parper
typedef struct csvPaper
{
    csvString fieldCache;
    char lineData[MAX_SIZE];
    csvString header[MAX_SIZE];
    csvString recodeCache[MAX_SIZE];
    FILE *file;
} csvPaper;

/**
 * 缓存清理
 * @param recode 被清除的地址
 * @param parser 解析器地址
*/
void cacheClear(csvString *recode, csvPaper *parser)
{
    //清除filecache，初始化用
    if ((recode == &parser->fieldCache) || (recode->stringCount == 0))
    {
        recode->stringCount = 1;
    }

    for (size_t i = 0; i < recode->stringCount; i++)
    {
        for (size_t j = 0; j < recode[i].strLength; j++)
        {
            recode[i].str[j] = '\0';
        }
        recode[i].strLength = 0;
    }

    recode->stringCount = 0;
}

/**
 * 打开文件
 * @param fileName 文件名
 * @param parser 解析器地址
 * @return 打开文件成功返回 1， 否则返回 0
*/
int csvInit(const char *fileName, csvPaper *parser)
{
    parser->file = NULL;

    cacheClear(&parser->fieldCache, parser);
    cacheClear(parser->recodeCache, parser);
    cacheClear(parser->header, parser);

    parser->file = fopen(fileName, "r");

    if (parser->file)
    {
        return 1;
    }

    return 0;
}

/**
 * 当前 fieldCache 缓存的内容压入 record 并清空 fieldCache
 * @param recode 被存入数据的数组
 * @param parser 解析器地址
*/
void FieldHandler(csvString *record, csvPaper *parser)
{
    for (size_t i = 0; i < parser->fieldCache.strLength; i++)
    {
        record[record->stringCount].str[record[record->stringCount].strLength++] = parser->fieldCache.str[i];
    }
    record->stringCount++;

    cacheClear(&parser->fieldCache, parser);
}

/**
 * 状态转换
 * @param ch 解码字符
 * @param quote_status 当前解码状态
 * @param recode 解码后的存储地
 * @param parser 解析器地址
 * @return 切换后的状态
*/
int StateAction(char ch, int quote_status, csvString *record, csvPaper *parser)
{
    // CSV_ENCLOSURE_NONE " CSV_ENCLOSURE_ENTER " CSV_ENCLOSURE_EXIT
    if ((COMMA == ch) && (CSV_ENCLOSURE_ENTER != quote_status))
    {
        FieldHandler(record, parser);
        return CSV_ENCLOSURE_NONE;
    }

    if (DQUOTE == ch)
    {
        if (CSV_ENCLOSURE_EXIT == quote_status)
        {
            parser->fieldCache.str[parser->fieldCache.strLength++] = ch;
            return CSV_ENCLOSURE_ENTER;
        }
        else
        {
            // 状态转移到下一个状态码
            // CSV_ENCLOSURE_NONE -> CSV_ENCLOSURE_ENTER
            // CSV_ENCLOSURE_ENTER -> CSV_ENCLOSURE_EXIT
            return quote_status + 1;
        }
    }

    parser->fieldCache.str[parser->fieldCache.strLength++] = ch;

    return quote_status % CSV_ENCLOSURE_EXIT; // if CSV_ENCLOSURE_EXIT, return CSV_ENCLOSURE_NONE
}

/**
 * 解码
 * @param recode 存入地址
 * @return 0 解码完成 1 未完成
 * @param parser 解析器地址
*/
int ParseRecord(csvString *record, csvPaper *parser)
{
    int quote_status = CSV_ENCLOSURE_NONE;

    while (fgets(parser->lineData, MAX_SIZE, parser->file))
    {
        // Parse line to record
        for (size_t i = 0; i < strlen(parser->lineData) - 1; i++)
        {
            quote_status = StateAction(parser->lineData[i], quote_status, record, parser);
        }

        // Handle last field
        if (CSV_ENCLOSURE_ENTER != quote_status) // Record finished
        {
            FieldHandler(record, parser);
            return 1;
        }
        // Record Cintinued, push <CR><LF>
        parser->fieldCache.str[parser->fieldCache.strLength++] = CR;
        parser->fieldCache.str[parser->fieldCache.strLength++] = LF;
    }

    return 0; // File End
}

/**
 * 打印
 * @param recode 打印开始地址
 * @param sequence 打印序列号
*/
void RecordHandler(csvString *record, int sequence)
{
    printf("Record %d: \n", sequence);

    for (size_t i = 0; i < record->stringCount; i++)
    {
        printf("%s || ", record[i].str);
    }

    printf("\n\n");
}

/**
 * 不可超过最大容量 MAX_SIZE
 * @param parser 解析器地址
 * @return csv中recode个数，不包含header，解析失败返回-1
*/
int Parse(csvPaper *parser)
{
    if (!ParseRecord(parser->header, parser))
    {
        return -1;
    }

    int recordCount = 0;

    while (ParseRecord(parser->recodeCache, parser))
    {
        recordCount++;
        RecordHandler(parser->recodeCache, recordCount);
        cacheClear(parser->recodeCache, parser);
    }

    return recordCount;
}

/**
 * 开始执行csv转换
 * @param fileName 文件名
*/
void csvRun(const char *fileName)
{
    csvPaper csv_Parser;

    if (csvInit(fileName, &csv_Parser))
    {
        printf("Record Number: ");
        printf("%d\n", Parse(&csv_Parser));
        fclose(csv_Parser.file);
    }
    else
    {
        printf("No file");
    }
}
