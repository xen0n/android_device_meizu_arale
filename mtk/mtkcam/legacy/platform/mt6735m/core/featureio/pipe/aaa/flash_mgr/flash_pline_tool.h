#pragma once

class FlashPline
{
public:
    int aePline2FlashAlgPline(PLine* p, const strAETable* pAE, int maxExp);


    FlashPline();
    ~FlashPline();
private:

    evSetting* pEvSetting;
};

class FlashPlineTool
{
public:
    static int searchAePlineIndex(int* ind, strAETable* pAE, int exp, int afe, int isp);
    static int flashAlgPlineWrite(const char* fname,  strAETable* pPline);
    static int aePlineWrite(const char* fname,  strAETable* pAE);

};