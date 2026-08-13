// Включаемый файл для MFC
#include <afxwin.h>

//===================================================== 
// Класс CMFHelloApp
// Наследуем от базового класса CWinApp главный
// класс приложения CMFHelloApp
//===================================================== 
class CMFHelloApp : public CWinApp
{
public:
	// Мы будем переопределять метод InitInstance,
	// предназначенный для инициализации приложения
	virtual BOOL InitInstance();
};
 
// Создаем объект приложение класса CMFHelloApp
CMFHelloApp MFHelloApp;
 
//===================================================== 
// Метод InitInstance класса CMFHelloApp
// Переопределяем виртуальный метод InitInstance
// класса CWinApp. Он вызывается каждый раз при запуске 
// приложения
//===================================================== 
BOOL CMFHelloApp::InitInstance()
{
	AfxMessageBox("Hello, MFC!");

	return FALSE;
}
