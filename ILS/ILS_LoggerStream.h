#ifndef ILS_LoggerStreamH
#define ILS_LoggerStreamH

#include <cstring>
#include <sstream>

#include "ILS_Logger.h"

//------------------------------------------------------------------------------
/// Тривиальный класс, для возможности потокового формирования сообщения в макросе ILS_LOG.
class TLoggerStream
{
	typedef std::string Msg;
	typedef std::string LogId;
	typedef void (ILogger::* TFuncPtr)(const Msg& msg, const LogId& id) const;
	mutable std::ostringstream out;  // Поток для накопления вывода.
	mutable std::string m_sSectId;
	mutable LogId id;
	const ILogger* m_pLogger;
	TFuncPtr m_pFunc;

	void PrintMessage(std::string& str, const char* msg, va_list marker) const
	{
		std::string buf; // дополнительный буффер, может пригодится, а может нет

		// Для отображение параметра типа "время" используется специальный ключ %t, для логов просто переводим его в %f
		// ради этого приходится копировать строку msg в отдельный редактируемый буффер buf
		const char* ct = strstr(msg, "%t");
		if (ct)
		{
			buf = msg;
			size_t index = 0;
			while (true)
			{
				/* Locate the substring to replace. */
				index = buf.find("%t", index);
				if (index == std::string::npos) break;

				/* Make the replacement. */
				buf.replace(index, 2, "%f");

				/* Advance index forward so the next iteration doesn't pick it up as well. */
				index += 2;
			}
			vsnprintf(str.data(), ILogger::max_msg_size, buf.c_str(), marker);
		}
		else
		{
			vsnprintf(str.data(), ILogger::max_msg_size, msg, marker);
		}
	}

public:
	/// Конструктор.
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc) : m_pLogger(pLogger), m_pFunc(pFunc) {}
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect) : m_pLogger(pLogger), m_pFunc(pFunc), m_sSectId(sect) {}
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect, unsigned int ind) : m_pLogger(pLogger), m_pFunc(pFunc), m_sSectId(sect + std::to_string(ind)) {}
	const TLoggerStream& operator()(const LogId& id, const char* msg, ...) const
	{
		std::string str(ILogger::max_msg_size, 0);
		try
		{
			va_list marker;
			va_start(marker, msg);
			PrintMessage(str, msg, marker);
			va_end(marker);
			out << str;
			//			logOut(str,id);
		}
		catch (...) {}
		return *this;
	}
	const TLoggerStream& SectBegin(const char* msg, ...) const
	{
		std::string str(ILogger::max_msg_size, 0);
		try
		{
			out << "SectionBegin " << m_sSectId << " ";
			va_list marker;
			va_start(marker, msg);
			PrintMessage(str, msg, marker);
			va_end(marker);
			out << str;
			//			logOut(str,id);
		}
		catch (...) {}
		return *this;
	}
	void SectCheck(const char* sect) const
	{
		if (m_sSectId != sect && m_pLogger)
		{
			m_pLogger->errOut("Ожидается окончание секции " + m_sSectId + " вместо указанной " + sect, id);
		}
	}

	void SectCheck(const char* sect, unsigned int ind) const
	{
		if (m_sSectId != (sect + std::to_string(ind)) && m_pLogger)
		{
			m_pLogger->errOut("Ожидается окончание секции " + m_sSectId + " вместо указанной " + (sect + std::to_string(ind)), id);
		}
	}

	const TLoggerStream& SectEnd(const char* msg, ...) const
	{
		std::string str(ILogger::max_msg_size, 0);
		try
		{
			out << "SectionEnd " << m_sSectId << " ";
			va_list marker;
			va_start(marker, msg);
			PrintMessage(str, msg, marker);
			va_end(marker);
			out << str;
			//			logOut(str,id);
		}
		catch (...) {}
		m_sSectId = "";
		return *this;
	}
	const char* SectId() const
	{
		return m_sSectId.c_str();
	}
	void Flush() const
	{
		(m_pLogger->*m_pFunc)(out.str(), id);
		out.str("");
	}
	/// Вывод в поток.
	template<class T> inline const TLoggerStream& operator<<(const T& t) const { out << t; return *this; }
	~TLoggerStream()
	{
		if (m_sSectId != "")
		{
			// Если m_sSectId!="" знаачит она не была начата, но не закончена, заканчиваем насильно
			out << "SectionEnd " << m_sSectId << " ";
		}
		else
		{
			(m_pLogger->*m_pFunc)(out.str(), id);
		}
	}
};

#endif  // ILS_LoggerStreamH
