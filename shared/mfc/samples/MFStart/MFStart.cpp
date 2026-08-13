// Включаемый файл для MFC
#include <afxwin.h>

//===================================================== 
// Класс CMFStartApp
// Наследуем от базового класса CWinApp главный
// класс приложения CMFStartApp
//===================================================== 
class CMFStartApp : public CWinApp
{
public:
	// Мы будем переопределять метод InitInstance,
	// предназначенный для инициализации приложения
	virtual BOOL InitInstance();
};
 
// Создаем объект приложение класса CMFStartApp
CMFStartApp MFStartApp;
 
//===================================================== 
// Класс CMFStartWindow
// Наследуем от базового класса CFrameWnd класс 
// CMFStartWindow. Он будет представлять главное 
// окно нашего приложения
//===================================================== 
class CMFStartWindow : public CFrameWnd
{
public:
	// Объявляем конструктор класса CMFStartWindow
	CMFStartWindow();
}; 

//===================================================== 
// Метод InitInstance класса CMFStartApp
// Переопределяем виртуальный метод InitInstance
// класса CWinApp. Он вызывается каждый раз при запуске 
// приложения
//===================================================== 
BOOL CMFStartApp::InitInstance()
{
	// Создаем объект класса CMFStartWindow
	m_pMainWnd = new CMFStartWindow();

	// Отображаем окно на экране. Параметр m_nCmdShow
	// определяет режим в котором оно будет отображаться
	m_pMainWnd -> ShowWindow(m_nCmdShow);

	// Обновляем содержимое окна
	m_pMainWnd -> UpdateWindow();
	return TRUE;
}

//===================================================== 
// Конструктор класса CMFStartWindow
//===================================================== 
CMFStartWindow::CMFStartWindow()
{ 
	// Создаем окно приложения, соответствующее 
	// данному объекту класса CMFStartWindow
	Create(NULL, "Hello MFC"); 
}
