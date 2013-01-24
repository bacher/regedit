#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#define MAX_KEY_LENGTH 256
#define MAX_PATH_LENGTH 2048

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProcedureRename(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProcedureFind(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProcedureAbout(HWND, UINT, WPARAM, LPARAM);

HINSTANCE hInst;
HWND hTV, hLV;
char szClassName[] = "RegEdit";
char szTitleName[] = "Редактор реестра";
HTREEITEM tiRoot, tiRoots[4];
HKEY hkRoots[4] = {HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS};
HKEY ghKey;
char gszName[MAX_KEY_LENGTH] = "";
int isRename;
byte isFindNext = 0;
byte cFinded;

struct node
{
	HTREEITEM hti;
	node *next;
};
struct nodeStr
{
	char szName[MAX_KEY_LENGTH];
	nodeStr *next;
};
node *nodeList;
nodeStr *nodePath;
///////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	hInst = hInstance;

	HWND hwnd;
	MSG msg;

	WNDCLASSEX wc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szClassName;
	wc.lpfnWndProc = WindowProcedure;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));//LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));//LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	if(!RegisterClassEx(&wc)) return 0;

	hwnd = CreateWindowEx(0, szClassName, szTitleName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 750, 400, HWND_DESKTOP, NULL, hInstance, NULL);

	ShowWindow(hwnd, nShowCmd);

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

////////
bool OpenKey(HTREEITEM ti, DWORD dwType)
{
	char szPath[MAX_PATH_LENGTH] = "";
	char szTemp[MAX_PATH_LENGTH];

	TV_ITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.pszText = szTemp;
	tvi.cchTextMax = MAX_KEY_LENGTH;

	HTREEITEM tiTempB, tiTemp = ti;
	
	while(!(tiRoot == (tiTempB = TreeView_GetParent(hTV, tiTemp))))
	{
		tvi.hItem = tiTemp;
		TreeView_GetItem(hTV, &tvi);
		strcat_s(szTemp, "\\");
		strcat_s(szTemp, szPath);
		strcpy_s(szPath, szTemp);
		tiTemp = tiTempB;
	}
	
	HKEY hkRoot;	

	for(int i=0; i<4; i++)
	{
		if(tiTemp == tiRoots[i])
		{
			hkRoot = hkRoots[i];
			break;
		}
	}

	if(!(RegOpenKeyEx(hkRoot, szPath, 0, dwType, &ghKey) == ERROR_SUCCESS)) return false;
	RegCloseKey(hkRoot);
	return true;
}

////////
bool ShowSubKeys(HTREEITEM ti)
{
	node *nodeTemp = nodeList;
	while(nodeTemp->next != NULL)
	{
		if(nodeTemp->next->hti == ti) return false;
		nodeTemp = nodeTemp->next;
	}
	node *nodeCurrent = new node;
	nodeCurrent->hti = ti;
	nodeCurrent->next = NULL;
	nodeTemp->next = nodeCurrent;

	DWORD dwKeyCount, dwSubKeyCount;
	char szName[MAX_KEY_LENGTH];
	TV_INSERTSTRUCT tvIS;
	HKEY hkTemp;

	tvIS.item.hItem = NULL;
	tvIS.item.cchTextMax = MAX_KEY_LENGTH;
	tvIS.hParent = ti;
	tvIS.hInsertAfter = TVI_LAST;
	tvIS.item.cChildren = 1;

	RegQueryInfoKey(ghKey, NULL, NULL, NULL, &dwKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	for(int i=0; i<(int)dwKeyCount; i++)
	{
		RegEnumKey(ghKey, i, szName, MAX_KEY_LENGTH);
		tvIS.item.pszText = szName;
		tvIS.item.mask = TVIF_TEXT;
		if(RegOpenKeyEx(ghKey, szName, 0, KEY_READ, &hkTemp) == ERROR_SUCCESS)
		{
			RegQueryInfoKey(hkTemp, NULL, NULL, NULL, &dwSubKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			if(dwSubKeyCount) tvIS.item.mask = TVIF_TEXT|TVIF_CHILDREN;
			RegCloseKey(hkTemp);
		}
		
		TreeView_InsertItem(hTV, &tvIS);
	}
	return true;
}

////////
bool ShowValues()
{
	ListView_DeleteAllItems(hLV);

	DWORD dwValueCount, dwValueSize, dwType, dwDataSize;
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	char szName[MAX_KEY_LENGTH];

	RegQueryInfoKey(ghKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL);
	for(int i=0; i<(int)dwValueCount; i++)
	{
		dwValueSize = MAX_KEY_LENGTH;
		RegEnumValue(ghKey, i, szName, &dwValueSize, NULL, &dwType, NULL, NULL);
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = szName;
		ListView_InsertItem(hLV, &lvi);
		lvi.iSubItem = 1;
		lvi.pszText = "Unknown";
		switch(dwType)
		{
		case REG_SZ:
			{
				lvi.pszText = "REG_SZ";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				dwDataSize = 1024;
				char szData[1024] = "";
				RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)szData, &dwDataSize);
				lvi.pszText = szData;

				break;
			}
		case REG_EXPAND_SZ:
			{
				lvi.pszText = "REG_EXPAND_SZ";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				lvi.pszText = "Unknown";
				dwDataSize = 1024;
				char szData[1024];
				if(RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
				{
					lvi.pszText = szData;
				}
				break;
			}
		case REG_MULTI_SZ:
			{
				lvi.pszText = "REG_MULTI_SZ";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				lvi.pszText = "Unknown";
				dwDataSize = 1024;
				char szData[1024];
				if(RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)szData, &dwDataSize) == ERROR_SUCCESS)
				{
					lvi.pszText = szData;
				}
				break;
			}
		case REG_DWORD:
			{
				lvi.pszText = "REG_DWORD";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				lvi.pszText = "Unknown";
				dwDataSize = sizeof(DWORD);
				DWORD dwData;
				if(RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)&dwData, &dwDataSize) == ERROR_SUCCESS)
				{
					char szBuff[16];
					char szText[32];
					_itoa_s(dwData, szBuff, 16);
					strcpy_s(szText, "0x");
					strcat_s(szText, szBuff);
					strcat_s(szText, " (");
					_itoa_s(dwData, szBuff, 10);
					strcat_s(szText, szBuff);
					strcat_s(szText, ")");
					lvi.pszText = szText;
				}
				break;
			}
		case REG_QWORD:
			{
				lvi.pszText = "REG_QWORD";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				lvi.pszText = "Unknown";
				dwDataSize = 8;
				DWORD qwData[2];
				if(RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)&qwData, &dwDataSize) == ERROR_SUCCESS)
				{
					DWORD dwData = qwData[0];
					char szBuff[16];
					char szText[32];
					_itoa_s(dwData, szBuff, 16);
					strcpy_s(szText, "0x");
					strcat_s(szText, szBuff);
					strcat_s(szText, " (");
					_itoa_s(dwData, szBuff, 10);
					strcat_s(szText, szBuff);
					strcat_s(szText, ")");
					lvi.pszText = szText;
				}
				break;
			}
		case REG_BINARY:
			{
				lvi.pszText = "REG_BINARY";
				ListView_SetItem(hLV, &lvi);
				lvi.iSubItem = 2;
				lvi.pszText = "Неизвестный тип данных";
			}
		}
		ListView_SetItem(hLV, &lvi);
	}
	return true;
}

////////
bool CreateValue(HWND hwnd, DWORD dwType)
{
	HTREEITEM tiSelected;

	tiSelected = TreeView_GetSelection(hTV);
	if(tiSelected == tiRoot) 
	{
		MessageBox(hwnd, "Нельзя создать запись в корневом разделе", szTitleName, MB_ICONWARNING);
		return false;
	}

	strcpy_s(gszName, "");
	isRename = false;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
	if(strcmp(gszName, "") != 0)
	{
		if(OpenKey(tiSelected, KEY_ALL_ACCESS))
		{
			if(RegQueryValueEx(ghKey, gszName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				MessageBox(hwnd, "Запись с таким именем уже существует", szTitleName, MB_ICONERROR);
				return false;
			}
			if(dwType == REG_DWORD)
			{
				DWORD dwData = 0;
				RegSetValueEx(ghKey, gszName, NULL, dwType, (LPBYTE)&dwData, sizeof(DWORD));
			}
			if(dwType == REG_SZ) RegSetValueEx(ghKey, gszName, NULL, dwType, NULL, NULL);
			RegCloseKey(ghKey);

			OpenKey(tiSelected, KEY_READ);
			ShowValues();
			RegCloseKey(ghKey);

			return true;
		}
		else
		{
			MessageBox(hwnd, "Нет прав на создание записи", szTitleName, MB_ICONERROR);
			return false;
		}
	}
	return false;
}

////////
bool FindRegistry(HKEY hKey, char *szParentName, char *szFind, bool isKey)
{
	HKEY hkTemp;
	bool isFinded = false;
	if(RegOpenKey(hKey, szParentName, &hkTemp) == ERROR_SUCCESS)
	{
		DWORD dwSubKeyCount, dwValueCount, dwValueSize;
		RegQueryInfoKey(hkTemp, NULL, NULL, NULL, &dwSubKeyCount, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL);
		char szName[MAX_KEY_LENGTH];
		if(!isKey)
		{
			for(int i=0; i<(int)dwValueCount; i++)
			{
				dwValueSize = MAX_KEY_LENGTH;
				RegEnumValue(hkTemp, i, szName, &dwValueSize, NULL, NULL, NULL, NULL);
				if(strcmp(szName, szFind) == 0)
				{
					cFinded++;
					if(cFinded > isFindNext) isFinded = true;
					break;
				}
			}
		}
		for(int i=0; i<(int)dwSubKeyCount; i++)
		{
			RegEnumKey(hkTemp, i, szName, MAX_KEY_LENGTH);
			if(isKey)
			{
				if(strcmp(szName, szFind) == 0)
				{
					cFinded++;
					if(cFinded > isFindNext)
					{
						isFinded = true;

						nodeStr *nodeCurrent = new nodeStr;
						nodeCurrent->next = nodePath;
						strcpy_s(nodeCurrent->szName, szName);
						nodePath = nodeCurrent;
						break;
					}
				}
			}

			if(FindRegistry(hkTemp, szName, szFind, isKey))
			{
				isFinded = true;

				nodeStr *nodeCurrent = new nodeStr;
				nodeCurrent->next = nodePath;
				strcpy_s(nodeCurrent->szName, szName);
				nodePath = nodeCurrent;
				break;
			}
		}
		RegCloseKey(hkTemp);
	}
	return isFinded;
}
////////////////
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			hTV = CreateWindowEx(0, WC_TREEVIEW, TEXT("Tree View"),
				WS_VISIBLE|WS_CHILD|WS_BORDER|TVS_HASLINES|TVS_HASBUTTONS,
				0, 0, 0, 0, hwnd, NULL, hInst, NULL);
			hLV = CreateWindow(WC_LISTVIEW, NULL,
				WS_VISIBLE|WS_CHILD|LVS_REPORT|WS_HSCROLL,
				0, 0, 0, 0, hwnd, NULL, hInst, NULL);

			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT|LVCF_WIDTH;
			lvc.pszText = "Название";
			lvc.cx = 150;
			SendMessage(hLV, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
			lvc.pszText = "Тип";
			lvc.cx = 120;											
			SendMessage(hLV, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);	
			lvc.pszText = "Информация";
			lvc.cx = 400; 
			SendMessage(hLV, LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);

			SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

			SendMessage(hwnd, WM_COMMAND, ID_FILE_REFRESH, 0);
			break;
		}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_FILE_REFRESH:
			{
				node *nodeDelete, *nodeTemp = nodeList;
				while(nodeTemp != NULL)
				{
					nodeDelete = nodeTemp;
					nodeTemp = nodeTemp->next;
					delete(nodeDelete);
				}
				nodeList = new node;

				nodeList->hti = NULL;
				nodeList->next = NULL;

				//isFindNext = 0;
				//
				nodeStr *nodeDeleteStr, *nodeTempStr = nodePath;
				while(nodeTempStr != NULL)
				{
					nodeDeleteStr = nodeTempStr;					// проверить
					nodeTempStr = nodeTempStr->next;
					delete(nodeDeleteStr);
				}
				nodePath = NULL;
				//
				TreeView_DeleteAllItems(hTV);
				ListView_DeleteAllItems(hLV);

				TV_INSERTSTRUCT tvIS;
				tvIS.item.mask = TVIF_TEXT|TVIF_CHILDREN;
				tvIS.item.hItem = NULL;
				tvIS.item.pszText = "[Компьютер]";
				tvIS.item.cchTextMax = MAX_KEY_LENGTH;
				tvIS.item.cChildren = 1;
				tvIS.hParent = TVI_ROOT;
				tvIS.hInsertAfter = TVI_LAST;
				tiRoot = TreeView_InsertItem(hTV, &tvIS);

				tvIS.hParent = tiRoot;
				tvIS.item.pszText = "HKEY_CLASSES_ROOT";
				tiRoots[0] = TreeView_InsertItem(hTV, &tvIS);
				tvIS.item.pszText = "HKEY_CURRENT_USER";
				tiRoots[1] = TreeView_InsertItem(hTV, &tvIS);
				tvIS.item.pszText = "HKEY_LOCAL_MACHINE";
				tiRoots[2] = TreeView_InsertItem(hTV, &tvIS);
				tvIS.item.pszText = "HKEY_USERS";
				tiRoots[3] = TreeView_InsertItem(hTV, &tvIS);
				
				TreeView_Expand(hTV, tiRoot, TVE_EXPAND);
				TreeView_Select(hTV, tiRoot, TVGN_CARET);

				break;
			}
		case ID_FILE_EXIT:
			{
				PostQuitMessage(0);
				break;
			}
		case ID_KEY_NEW:
			{
				HTREEITEM tiSelected;
				tiSelected = TreeView_GetSelection(hTV);
				if(tiSelected == tiRoot)
				{
					MessageBox(hwnd, "Нельзя создать раздел в корне", szTitleName, MB_ICONERROR);
					break;
				}

				if(OpenKey(tiSelected, KEY_ALL_ACCESS))
				{
					strcpy_s(gszName, "");
					isRename = false;
					DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
					if(strcmp(gszName, "") != 0)
					{
						HKEY hkTemp;
						if(RegOpenKeyEx(ghKey, gszName, 0, KEY_READ, &hkTemp) != ERROR_SUCCESS)
						{
							HKEY hkNew;
							RegCreateKey(ghKey, gszName, &hkNew);
							RegCloseKey(hkNew);

							TV_INSERTSTRUCT tvIS;
							tvIS.item.mask = TVIF_TEXT;
							tvIS.item.hItem = NULL;
							tvIS.item.pszText = gszName;
							tvIS.item.cchTextMax = MAX_KEY_LENGTH;
							tvIS.hParent = tiSelected;
							tvIS.hInsertAfter = TVI_LAST;
							TreeView_InsertItem(hTV, &tvIS);
							//
							node *nodeTemp = nodeList;
							while(nodeTemp->next != NULL) nodeTemp = nodeTemp->next;
							node *nodeCurrent = new node;
							nodeCurrent->hti = tiSelected;
							nodeCurrent->next = NULL;
							nodeTemp->next = nodeCurrent;
							//
							TV_ITEM tvi;
							tvi.mask = TVIF_CHILDREN;
							tvi.hItem = tiSelected;
							tvi.cChildren = 1;
							TreeView_SetItem(hTV, &tvi);
						}
						else
						{
							MessageBox(hwnd, "Раздел с таким именем уже существует", szTitleName, MB_ICONERROR);
							RegCloseKey(hkTemp);
						}
					}
					RegCloseKey(ghKey);
				}
				else MessageBox(hwnd, "Нет прав на создание раздела", szTitleName, MB_ICONERROR);

				break;
			}
		case ID_KEY_RENAME:
			{
				HTREEITEM tiSelected, tiParent;
				tiSelected = TreeView_GetSelection(hTV);
				tiParent = TreeView_GetParent(hTV, tiSelected);
				if(tiSelected == tiRoot || tiParent == tiRoot)
				{
					MessageBox(hwnd, "Нельзя переименовать корневой раздел", szTitleName, MB_ICONERROR);
					break;
				}

				char szName[MAX_KEY_LENGTH];
				TV_ITEM tvi;
				tvi.mask = TVIF_TEXT;
				tvi.pszText = szName;
				tvi.cchTextMax = MAX_KEY_LENGTH;
				tvi.hItem = tiSelected;
				TreeView_GetItem(hTV, &tvi);

				strcpy_s(gszName, szName);
				isRename = true;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
				if(strcmp(gszName, szName) != 0)
				{
					if(OpenKey(tiParent, KEY_ALL_ACCESS))
					{
						HKEY hkTemp;
						if(RegOpenKeyEx(ghKey, gszName, 0, KEY_READ, &hkTemp) != ERROR_SUCCESS)
						{
							HKEY hkNew;
							RegCreateKey(ghKey, gszName, &hkNew);
							RegCopyTree(ghKey, szName, hkNew);
							RegDeleteTree(ghKey, szName);
							RegCloseKey(ghKey);
							RegCloseKey(hkNew);

							tvi.pszText = gszName;
							TreeView_SetItem(hTV, &tvi);
						}
						else
						{
							MessageBox(hwnd, "Раздел с таким именем уже существует", szTitleName, MB_ICONERROR);
							RegCloseKey(hkTemp);
						}
					}
					else MessageBox(hwnd, "Нет прав на изменение раздела", szTitleName, MB_ICONERROR);
				}
				break;
			}
		case ID_KEY_DELETE:
			{
				HTREEITEM tiSelected, tiParent;

				tiSelected = TreeView_GetSelection(hTV);
				tiParent = TreeView_GetParent(hTV, tiSelected);
				if(tiSelected == tiRoot || tiParent == tiRoot)
				{
					MessageBox(hwnd, "Нельзя удалить корневой раздел", szTitleName, MB_ICONERROR);
					break;
				}
				if(MessageBox(hwnd, "Вы действительн хотите удалить выбранный раздел и всё что внутри?", szTitleName, MB_YESNO|MB_ICONWARNING) == IDYES)
				{
					if(OpenKey(tiParent, KEY_READ))
					{
						char szSelected[MAX_KEY_LENGTH];
						TV_ITEM tvi;
						tvi.mask = TVIF_TEXT;
						tvi.pszText = szSelected;
						tvi.cchTextMax = MAX_KEY_LENGTH;
						tvi.hItem = tiSelected;
						TreeView_GetItem(hTV, &tvi);

						RegDeleteTree(ghKey, szSelected);
						RegCloseKey(ghKey);
						TreeView_DeleteItem(hTV, tiSelected);
					}
				}
				break;
			}
		case ID_VALUE_MODIFY:
			{
				int iSel = ListView_GetSelectionMark(hLV);
				if(iSel != -1)
				{
					char szName[MAX_KEY_LENGTH];
					ListView_GetItemText(hLV, iSel, 0, szName, MAX_KEY_LENGTH);

					HTREEITEM tiSelected;
					tiSelected = TreeView_GetSelection(hTV);
					if(OpenKey(tiSelected, KEY_ALL_ACCESS))
					{
						DWORD dwType, dwDataSize;
						RegQueryValueEx(ghKey, szName, NULL, &dwType, NULL, NULL);
						if(dwType == REG_SZ)
						{
							dwDataSize = 1024;
							char szData[1024];
							RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)szData, &dwDataSize);
							strcpy_s(gszName, "");
							if(dwDataSize != 0) strcpy_s(gszName, szData);
							isRename = 2;
							DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
							if(strcmp(gszName, szData) != 0)
							{
								RegSetValueEx(ghKey, szName, NULL, dwType, (LPBYTE)gszName, strlen(gszName)*sizeof(char));
							}
						}
						else if(dwType == REG_DWORD)
						{
							dwDataSize = sizeof(DWORD);
							DWORD dwData;
							RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)&dwData, &dwDataSize);
							char szData[16];
							_itoa_s(dwData, szData, 10);
							strcpy_s(gszName, szData);
							isRename = 2;
							DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
							if(strcmp(gszName, szData) != 0)
							{
								dwData = atoi(gszName);
								RegSetValueEx(ghKey, szName, NULL, dwType, (LPBYTE)&dwData, sizeof(DWORD));
							}
						}
						else MessageBox(hwnd, "Неизвестный формат, редактирование невозможно", szTitleName, MB_ICONERROR);
						RegCloseKey(ghKey);

						OpenKey(tiSelected, KEY_READ);
						ShowValues();
						RegCloseKey(ghKey);
					}
					else MessageBox(hwnd, "Нет прав на изменение записи", szTitleName, MB_ICONERROR);
				}
				else MessageBox(hwnd, "Запись не выбрана", szTitleName, MB_ICONINFORMATION);
				break;
			}
		case ID_VALUE_NEW_STRING:
			{
				CreateValue(hwnd, REG_SZ);
				break;
			}
		case ID_VALUE_NEW_DWORD:
			{
				CreateValue(hwnd, REG_DWORD);
				break;
			}
		case ID_VALUE_RENAME:
			{
				int iSel = ListView_GetSelectionMark(hLV);
				if(iSel != -1)
				{
					HTREEITEM tiSelected;
					tiSelected = TreeView_GetSelection(hTV);
					if(OpenKey(tiSelected, KEY_ALL_ACCESS))
					{
						DWORD dwDataSize, dwDataType;
						byte *temp;
						char szName[MAX_KEY_LENGTH];
						ListView_GetItemText(hLV, iSel, 0, szName, MAX_KEY_LENGTH);
						RegQueryValueEx(ghKey, szName, NULL, &dwDataType, NULL, &dwDataSize);
						temp = new byte[dwDataSize];
						RegQueryValueEx(ghKey, szName, NULL, NULL, (LPBYTE)temp, &dwDataSize);

						strcpy_s(gszName, szName);
						isRename = true;
						DialogBox(hInst, MAKEINTRESOURCE(IDD_RENAME), hwnd, DialogProcedureRename);
						if(strcmp(gszName, szName) != 0)
						{
							if(RegQueryValueEx(ghKey, gszName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
							{
								MessageBox(hwnd, "Запись с таким именем уже существует", szTitleName, MB_ICONERROR);
								break;
							}
							RegDeleteValue(ghKey, szName);
							RegSetValueEx(ghKey, gszName, NULL, dwDataType, (LPBYTE)temp, dwDataSize);

							OpenKey(tiSelected, KEY_READ);
							ShowValues();
						}
						RegCloseKey(ghKey);
					}
					else MessageBox(hwnd, "Нет прав на изменение записи", szTitleName, MB_ICONERROR);
				}
				else MessageBox(hwnd, "Запись не выбрана", szTitleName, MB_ICONINFORMATION);
				break;
			}
		case ID_VALUE_DELETE:
			{
				int iSel = ListView_GetSelectionMark(hLV);
				if(iSel != -1)
				{
					if(MessageBox(hwnd, "Вы действительно хотите удалить запись?", szTitleName, MB_YESNO|MB_ICONWARNING) == IDYES)
					{
						HTREEITEM tiSelected;
						tiSelected = TreeView_GetSelection(hTV);
						OpenKey(tiSelected, KEY_ALL_ACCESS);
						char szName[MAX_KEY_LENGTH];
						ListView_GetItemText(hLV, iSel, 0, szName, MAX_KEY_LENGTH);
						if(RegDeleteValue(ghKey, szName) != ERROR_SUCCESS) MessageBox(hwnd, "Запись нельзя удалить", szTitleName, MB_ICONERROR);
						RegCloseKey(ghKey);
						
						ListView_DeleteItem(hLV, iSel);
					}
				}
				else MessageBox(hwnd, "Запись не выбрана", szTitleName, MB_ICONWARNING);
				break;
			}
		case ID_FIND_FIND:
			{
				HTREEITEM tiSelected;
				tiSelected = TreeView_GetSelection(hTV);
				if(tiSelected != tiRoot)
				{
					isFindNext = 0;
					DialogBox(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, DialogProcedureFind);
				}
				else
				{
					MessageBox(hwnd, "Выберите раздел", szTitleName, MB_ICONWARNING);
				}
				break;
			}
		case ID_FIND_FINDNEXT:
			{
				if(nodePath != NULL)
				{
					isFindNext++;
					DialogBox(hInst, MAKEINTRESOURCE(IDD_FIND), hwnd, DialogProcedureFind);
					//isFindNext = false;
				}
				else
				{
					SendMessage(hwnd, WM_COMMAND, ID_FIND_FIND, 0);
				}
				break;
			}
		case ID_HELP_ABOUT:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, DialogProcedureAbout);
				break;
			}
		case ID_ACCEL_RENAME:
			{
				HWND hFocus = GetFocus();
				if(hFocus == hTV) SendMessage(hwnd, WM_COMMAND, ID_KEY_RENAME, 0);
				else if(hFocus == hLV) SendMessage(hwnd, WM_COMMAND, ID_VALUE_RENAME, 0);
				else if(hFocus == hwnd) MessageBox(hwnd, "Необходимо выбрать объект", szTitleName, MB_ICONWARNING);
				break;
			}
		case ID_ACCEL_DELETE:
			{
				HWND hFocus = GetFocus();
				if(hFocus == hTV) SendMessage(hwnd, WM_COMMAND, ID_KEY_DELETE, 0);
				else if(hFocus == hLV) SendMessage(hwnd, WM_COMMAND, ID_VALUE_DELETE, 0);
				else if(hFocus == hwnd) MessageBox(hwnd, "Необходимо выбрать объект", szTitleName, MB_ICONWARNING);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case TVN_ITEMEXPANDING:
			{
				LPNMTREEVIEW ptvi = (LPNMTREEVIEW)lParam;
				if(ptvi->itemNew.hItem == tiRoot) break;
				if(!OpenKey(ptvi->itemNew.hItem, KEY_READ))
				{
					MessageBox(hwnd, "Не удалось открыть раздел", szTitleName, MB_ICONWARNING);
					break;
				}
				ShowSubKeys(ptvi->itemNew.hItem);
				RegCloseKey(ghKey);
				break;
			}
		case TVN_SELCHANGING:
			{
				LPNMTREEVIEW ptvi = (LPNMTREEVIEW)lParam;
				if(ptvi->itemNew.hItem == tiRoot)
				{
					ListView_DeleteAllItems(hLV);
					break;
				}
				if(!OpenKey(ptvi->itemNew.hItem, KEY_READ))
				{
					MessageBox(hwnd, "Не удалось открыть раздел", szTitleName, MB_ICONWARNING);
					break;
				}
				ShowValues();
				RegCloseKey(ghKey);
				break;
			}
		case NM_RCLICK:
			{
				POINT p;
				GetCursorPos(&p);
				int iSel = -1;

				if(((LPNMHDR)lParam)->hwndFrom == hTV)
				{
					static HMENU hPopupMenu = GetSubMenu(GetMenu(hwnd), 1);
					iSel = TrackPopupMenu(hPopupMenu, TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hTV, NULL);
					SendMessage(hwnd, WM_COMMAND, iSel, 0);
				}
				if(((LPNMHDR)lParam)->hwndFrom == hLV)
				{
					static HMENU hPopupMenu = GetSubMenu(GetMenu(hwnd), 2);
					iSel = TrackPopupMenu(hPopupMenu, TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hLV, NULL);
					SendMessage(hwnd, WM_COMMAND, iSel, 0);
				}
				break;
			}
		}
		break;
	case WM_SIZE:
		{
			MoveWindow(hTV, -1, -1, 301, HIWORD(lParam)+2, true);
			MoveWindow(hLV, 300, 0, LOWORD(lParam)-300, HIWORD(lParam), true);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
////////
BOOL CALLBACK DialogProcedureRename(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND editName;
    switch(msg)
    {
	case WM_INITDIALOG:
		{
			editName = GetDlgItem(hDlg, IDC_NAME);
			if(isRename)
			{
				SetWindowText(editName, gszName);
				if(isRename == 2)
				{
					SetWindowText(hDlg, "Редактирование данных");
					SetWindowText(GetDlgItem(hDlg, IDC_LABEL), "Значение:");
				}
			}
			else SetWindowText(hDlg, "Создание раздела");
			SetFocus(editName);
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
            {
			case IDOK:
				{
					char szName[MAX_KEY_LENGTH];
					GetWindowText(editName, szName, MAX_KEY_LENGTH);
					if(strlen(szName))
					{
						if(strcmp(gszName, szName) != 0) strcpy_s(gszName, szName);
						EndDialog(hDlg, 0);
						return false;
					}
					else MessageBox(hDlg, "Заполните поле", szTitleName, MB_ICONINFORMATION);
					break;
				}
			case IDCANCEL:
				{
                    EndDialog(hDlg, 0);
                    return false;
				}
            }
			break;
		}
		break;
    }
    return false;
}

////////
BOOL CALLBACK DialogProcedureFind(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND editFind, radioKey, radioValue;
	static bool isKey;
	static HTREEITEM tiSelected;
	static char szFind[MAX_KEY_LENGTH];
    switch(msg)
    {
	case WM_INITDIALOG:
		{
			editFind = GetDlgItem(hDlg, IDC_FIND);
			radioKey = GetDlgItem(hDlg, IDC_KEY);
			radioValue = GetDlgItem(hDlg, IDC_VALUE);
			SendMessage(radioKey, BM_SETCHECK, 1, 0);
			SetFocus(editFind);
			if(isFindNext) SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
            {
			case IDOK:
				{
					//bool isKey;
					//char szFind[MAX_KEY_LENGTH];
					if(!isFindNext)
					{
						isKey = false;
						if(SendMessage(radioKey, BM_GETSTATE, 0, 0)) isKey = true;
						GetWindowText(editFind, szFind, MAX_KEY_LENGTH);
					}
					if(strlen(szFind))
					{
						//
						nodeStr *nodeDeleteStr, *nodeTempStr = nodePath;
						while(nodeTempStr != NULL)
						{
							nodeDeleteStr = nodeTempStr;
							nodeTempStr = nodeTempStr->next;
							delete(nodeDeleteStr);
						}
						nodePath = NULL;

						EnableWindow(radioKey, false);
						EnableWindow(radioValue, false);
						EnableWindow(editFind, false);
						EnableWindow(GetDlgItem(hDlg, IDOK), false);
						
						if(!isFindNext) tiSelected = TreeView_GetSelection(hTV);

						if(OpenKey(tiSelected, KEY_READ))
						{
							char szName[MAX_KEY_LENGTH] = "";
							cFinded = 0;
							if(FindRegistry(ghKey, szName, szFind, isKey))
							{
								nodeStr *nodeTemp = nodePath;
								HTREEITEM tiTemp = tiSelected;
								while(nodeTemp != NULL)
								{
									//MessageBox(hDlg, nodeTemp->szName, "", NULL);
									//
									OpenKey(tiTemp, KEY_READ);
									ShowSubKeys(tiTemp);

									HTREEITEM tiChild;
									//int cChild;
									//tiChild = TreeView_GetChild(hTV, tiSelected);
									char szTemp[MAX_KEY_LENGTH];
									TV_ITEM tvi;
									tvi.mask = TVIF_TEXT;
									tvi.pszText = szTemp;
									tvi.cchTextMax = MAX_KEY_LENGTH;
									tiChild = TreeView_GetNextItem(hTV, tiTemp, TVGN_CHILD);
									while(tiChild)
									{
										tvi.hItem = tiChild;
										TreeView_GetItem(hTV, &tvi);
										//MessageBox(hDlg, szTemp, "", NULL);
										if(strcmp(szTemp, nodeTemp->szName) == 0)
										{
											tiTemp = tiChild;
											break;
										}
										tiChild = TreeView_GetNextItem(hTV, tiChild, TVGN_NEXT);
									}
									//
									nodeTemp = nodeTemp->next;
								}
								TreeView_Select(hTV, tiTemp, TVGN_CARET);
								
								//RegCloseKey(ghKey);
								EndDialog(hDlg, 1);
								SetFocus(hTV);
								return true;
							}
							else
							{
								EnableWindow(radioKey, true);
								EnableWindow(radioValue, true);
								EnableWindow(editFind, true);
								EnableWindow(GetDlgItem(hDlg, IDOK), true);
								MessageBox(hDlg, "Ничего не найдено", szTitleName, MB_ICONINFORMATION);
								if(isFindNext)
								{
									EndDialog(hDlg, 0);
									return true;
								}
								//RegCloseKey(ghKey);
							}
						}
						else
						{
							MessageBox(hDlg, "Нет доступа", szTitleName, MB_ICONERROR);
							EndDialog(hDlg, 0);
							return true;
						}
					}
					else MessageBox(hDlg, "Заполните поле", szTitleName, MB_ICONINFORMATION);
					break;
				}
			case IDCANCEL:
				{
                    EndDialog(hDlg, 0);
                    return true;
				}
            }
			break;
		}
		break;
    }
    return false;
}
////////
BOOL CALLBACK DialogProcedureAbout(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
            {
			case IDOK:
				{
					EndDialog(hDlg, 0);
					return false;
				}
			case IDCANCEL:
				{
					EndDialog(hDlg, 0);
					return false;
				}
            }
			break;
		}
		break;
    }
    return false;
}