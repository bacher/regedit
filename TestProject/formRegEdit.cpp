//---------------------------------------------------------------------------
#include <vcl.h>
#include <winreg.h>
#pragma hdrstop

#include "formRegEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define MB(s) MessageBoxA(NULL, s, szAppName, MB_ICONINFORMATION)

const char szAppName[] = "RegEdit";
TfrmRegEdit *frmRegEdit;
//---------------------------------------------------------------------------
__fastcall TfrmRegEdit::TfrmRegEdit(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn1Click(TObject *Sender)
{
    // Открытие ключа
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    // Создание значений
    // REG_DWORD
    char szValueName[] = "value1";
    DWORD dwType = REG_DWORD;
    DWORD dwData = 34;
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)&dwData, sizeof(dwData));
    if(iRes != ERROR_SUCCESS) MB("Error2");

    // REG_EXPAND_SZ
    strcpy(szValueName, "value2");
    dwType = REG_EXPAND_SZ;
    char szData[64] = "Hello World";
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)szData, sizeof(szData));
    if(iRes != ERROR_SUCCESS) MB("Error3");

    // REG_MULTI_SZ
    strcpy(szValueName, "value3");
    dwType = REG_MULTI_SZ;
    strcpy(szData, "Hello World");
    strcpy(szData + strlen(szData) + 1, "Lolol");
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)szData, 17);
    if(iRes != ERROR_SUCCESS) MB("Error4");

    // REG_QWORD
    strcpy(szValueName, "value4");
    dwType = REG_QWORD;
    DWORD dwData2[2] = { 1, 2 };
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)dwData2, sizeof(DWORD) * 2);
    if(iRes != ERROR_SUCCESS) MB("Error4");

    // REG_SZ
    strcpy(szValueName, "value5");
    dwType = REG_SZ;
    strcpy(szData, "SZ VALUE");
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)szData, sizeof(szData));
    if(iRes != ERROR_SUCCESS) MB("Error3");

    // REG_BINARY
    strcpy(szValueName, "value6");
    dwType = REG_BINARY;
    strcpy(szData, "SZ VALUE");
    iRes = RegSetValueEx(hkOut, szValueName, 0, dwType, (BYTE*)szData, sizeof(szData));
    if(iRes != ERROR_SUCCESS) MB("Error3");


    iRes = RegCloseKey(hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error0");
}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn2Click(TObject *Sender)
{
    // Получить количество подключей в ключе
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    DWORD dwKeyCount;
    RegQueryInfoKey(hkOut, NULL, NULL, NULL, &dwKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    MB(IntToStr(dwKeyCount).c_str());
    
    RegCloseKey(hkOut);
}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn3Click(TObject *Sender)
{
    // Прочесть подключи в ключе
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    char szName[64];
    RegEnumKey(hkOut, 0, szName, 64);
    MB(szName);

    RegCloseKey(hkOut);
}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn4Click(TObject *Sender)
{
    // получить имя значения
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    char szName[64];
    DWORD dwValueSize, dwType;
    RegEnumValue(hkOut, 0, szName, &dwValueSize, NULL, &dwType, NULL, NULL);
    MB(szName);

    RegCloseKey(hkOut);
}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn5Click(TObject *Sender)
{
    // получить параметр значения
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    char szData[64];
    DWORD dwValueSize = 64;
    RegQueryValueEx(hkOut, "value5", NULL, NULL, (BYTE*)szData, &dwValueSize);
    MB(szData);

    RegCloseKey(hkOut);

}
//---------------------------------------------------------------------------
void __fastcall TfrmRegEdit::btn6Click(TObject *Sender)
{
    // создание ключа
    int iRes;
    HKEY hkRootCU = HKEY_CURRENT_USER;
    HKEY hkOut;
    char szSubKey[] = "Software\\Bacher";
    iRes = RegOpenKeyEx(hkRootCU, szSubKey, 0, KEY_ALL_ACCESS, &hkOut);
    if(iRes != ERROR_SUCCESS) MB("Error1");

    HKEY hkNewKey;
    RegCreateKey(hkOut, "newkey", &hkNewKey);

    RegCloseKey(hkNewKey);
    RegCloseKey(hkOut);
}
//---------------------------------------------------------------------------

/*
RegCopyTree(ghKey, szName, hkNew);
RegDeleteTree(ghKey, szName);
RegDeleteValue(ghKey, szName);
*/
