#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\PAIConfigurationManager.h"
#include ".\Common\NHDbgApi.h"

CPAIConfigurationManager::CPAIConfigurationManager(void) {}
CPAIConfigurationManager::~CPAIConfigurationManager(void) {}

bool CPAIConfigurationManager::GetConfiguration(PAIConfiguration &config) const
{
    CStringA filename(PAI_CONFIG_INI);
    return LoadConfigurationFromIni(filename.GetBuffer(), config);

    NVDump('F', 'C', "1O", L"PAI_CONF", L"NG_0");
    return false;
}

bool CPAIConfigurationManager::SaveConfiguration(const PAIConfiguration &config) const
{
    CStringA filename(PAI_CONFIG_INI);
    return SaveConfigurationToIni(filename.GetBuffer(), config);
}

CString CPAIConfigurationManager::GetProcessor(const PAIConfiguration &config) const
{
    switch (config.Processor)
    {
    case PAIPROC_FIS:
        return "FIS";
    case PAIPROC_SWITCH_COMMERCE:
        return "Switch Commerce";
    case PAIPROC_1ISO:
        return "1st ISO";
    case PAIPROC_COLUMBUS_DATA:
        return "Columbus Data (CDS)";
    case PAIPROC_OTHER:
        return "Other";
    default:
        return L"Unknown";
    }
};

CString CPAIConfigurationManager::GetMessageFormat(const PAIConfiguration &config) const
{
    switch (config.Message)
    {
    case PAIMSG_STANDARD1:
        return "Standard 1";
    case PAIMSG_STANDARD3:
        return "Standard 3";
    default:
        return L"Unknown";
    }
};

CString CPAIConfigurationManager::GetRMS(const PAIConfiguration &config) const
{
    switch (config.RMS)
    {
    case PAIRMS_NONE:
        return "No RMS";
    case PAIRMS_WIRELESS:
        return "PAI Wireless RMS";
    case PAIRMS_INTERNET:
        return "PAI Internet RMS";
    case PAIRMS_OTHER:
        return "Other RMS";
    default:
        return L"Unknown";
    }
};

//
// Private Functions
//

bool CPAIConfigurationManager::LoadConfigurationFromIni(char *filename, PAIConfiguration &config) const
{
    char *buffer = NULL;
    bool failed = false;
    int res = 0;

    // Processor
    res = GetIniString(filename, "Processor", "option", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.Processor = (PAIPROCESSOR)atoi(buffer);
        delete[] buffer;
    }

    // Communication
    res = GetIniString(filename, "Communication", "option", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.Communication = (PAICOMMUNICATION)atoi(buffer);
        delete[] buffer;
    }

    // Message
    res = GetIniString(filename, "Message", "option", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.Message = (PAIMESSAGEFORMAT)atoi(buffer);
        delete[] buffer;
    }

    // RMS
    res = GetIniString(filename, "RMS", "option", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.RMS = (PAIREMOTEMONITORING)atoi(buffer);
        delete[] buffer;
    }

    // Revenue Add Features 1
    res = GetIniString(filename, "RAF1", "dual_balance", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.DualBalance = (PAIENABLEMENT)atoi(buffer);
        delete[] buffer;
    }

    res = GetIniString(filename, "RAF1", "mastercard_cash_pickup", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.MastercardCashPickup = (PAIENABLEMENT)atoi(buffer);
        delete[] buffer;
    }

    res = GetIniString(filename, "RAF1", "popmoney", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.Popmoney = (PAIENABLEMENT)atoi(buffer);
        delete[] buffer;
    }

    res = GetIniString(filename, "RAF1", "fis_dcc", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.FIS_DCC = (PAIENABLEMENT)atoi(buffer);
        delete[] buffer;
    }

    // Revenue Add Features 2
    res = GetIniString(filename, "RAF2", "bitload4u", &buffer);
    failed |= res == -1;

    if (res == 1)
    {
        config.Bitload4U = (PAIENABLEMENT)atoi(buffer);
        delete[] buffer;
    }

    return !failed;
}

bool CPAIConfigurationManager::SaveConfigurationToIni(char *filename, const PAIConfiguration &config) const
{
    char buffer[256] = {};

    // Processor
    sprintf_s(buffer, sizeof(buffer), "%d", config.Processor);
    SetIniString(filename, "Processor", "option", buffer);
    memset(buffer, 0, sizeof(buffer));

    // Communication
    sprintf_s(buffer, sizeof(buffer), "%d", config.Communication);
    SetIniString(filename, "Communication", "option", buffer);
    memset(buffer, 0, sizeof(buffer));

    // Message
    sprintf_s(buffer, sizeof(buffer), "%d", config.Message);
    SetIniString(filename, "Message", "option", buffer);
    memset(buffer, 0, sizeof(buffer));

    // RMS
    sprintf_s(buffer, sizeof(buffer), "%d", config.RMS);
    SetIniString(filename, "RMS", "option", buffer);
    memset(buffer, 0, sizeof(buffer));

    // Revenue Add Features 1
    sprintf_s(buffer, sizeof(buffer), "%d", config.DualBalance);
    SetIniString(filename, "RAF1", "dual_balance", buffer);
    memset(buffer, 0, sizeof(buffer));

    sprintf_s(buffer, sizeof(buffer), "%d", config.MastercardCashPickup);
    SetIniString(filename, "RAF1", "mastercard_cash_pickup", buffer);
    memset(buffer, 0, sizeof(buffer));

    sprintf_s(buffer, sizeof(buffer), "%d", config.Popmoney);
    SetIniString(filename, "RAF1", "popmoney", buffer);
    memset(buffer, 0, sizeof(buffer));

    sprintf_s(buffer, sizeof(buffer), "%d", config.FIS_DCC);
    SetIniString(filename, "RAF1", "fis_dcc", buffer);
    memset(buffer, 0, sizeof(buffer));

    // Revenue Add Features 2
    sprintf_s(buffer, sizeof(buffer), "%d", config.Bitload4U);
    SetIniString(filename, "RAF2", "bitload4u", buffer);
    memset(buffer, 0, sizeof(buffer));

    return true;
}