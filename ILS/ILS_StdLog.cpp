#include <stdio.h>
#include <time.h>
#include <string.h>
#include "ILS_StdLog.h"

//=============================================================================
// BaseLogger - стандартая реализация основных механизмов Logger-а. 
// Рекомендуется наследоваться именно от этого класса при определении 
// собственного механизма регистрации хода процесса.
//-----------------------------------------------------------------------------
// Конструктор
BaseLogger::BaseLogger() {
	// По умолчанию ничего не выводим в заголовке
	show_info = 0;
	// Время начала
	bStarted = false;
	// флаг вывода лога в консоль
	bLogToConsole = false;
};
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::infOut(const Msg& msg, const LogId& id) const {
	lOut(iTitle() + msg);
}
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::logOut(const Msg& msg, const LogId& id) const {
	lOut(lTitle()+msg);
}
// Регистрация предупреждения (warning) и не фатальной ошибки
void BaseLogger::wrnOut(const Msg& msg, const LogId& id) const {
	++warnings;
	wOut(wTitle()+msg);
}
// Регистрация фатальной ошибки, 
// после которой результаты процесса не определены
void BaseLogger::errOut(const Msg& msg, const LogId& id) const {
	++errors;
	eOut(eTitle()+msg);
}
//-----------------------------------------------------------------------------
// Вспомогательные функции
// Создается общая для всех типов сообщений строка с заголовком
// на основе значений настроек
std::string BaseLogger::title() const {
	std::string res = "";
	char s[128];
	time_t ltime;
	time( &ltime );
	struct tm *today;
	today = localtime( &ltime );
	if(show_info & 1) {
		strftime(s, 128, "%Y/%m/%d ", today );
		res += s;
	}
	if((show_info & 2) || ((show_info & 4) && (!bStarted))) {
		strftime(s, 128, "%H:%M:%S ", today );
		res += s;
	}
	if (!bStarted) {
		start_time = std::chrono::steady_clock::now();
		bStarted = true;
	}
	else if (show_info & 4) {
		long fDurationMilli = long(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
		sprintf(s, "% 8.2f ", double(fDurationMilli) / 1000.0);
		res += s;
	}
	return res;
}
// Для кажного типа сообзения задается отдельная функция, которая 
// реализуется на основе общей
std::string BaseLogger::lTitle() const {
	return title()+"> ";
}
std::string BaseLogger::iTitle() const {
	return title() + "|INFO> ";
}
std::string BaseLogger::wTitle() const {
	return title()+"|WARNING> ";
}
std::string BaseLogger::eTitle() const {
	return title()+"|ERROR> ";
}
