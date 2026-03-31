


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined(USYNERGY_LITTLE_ENDIAN) && defined(USYNERGY_BIG_ENDIAN)

	#error "Can't define both USYNERGY_LITTLE_ENDIAN and USYNERGY_BIG_ENDIAN"
#elif !defined(USYNERGY_LITTLE_ENDIAN) && !defined(USYNERGY_BIG_ENDIAN)

	#if defined(__LITTLE_ENDIAN__) || defined(LITTLE_ENDIAN) || (_BYTE_ORDER == _LITTLE_ENDIAN)
		#define USYNERGY_LITTLE_ENDIAN
	#elif defined(__BIG_ENDIAN__) || defined(BIG_ENDIAN) || (_BYTE_ORDER == _BIG_ENDIAN)
		#define USYNERGY_BIG_ENDIAN
	#else
		#error "Can't detect endian-nes, please defined either USYNERGY_LITTLE_ENDIAN or USYNERGY_BIG_ENDIAN";
	#endif
#else

#endif


typedef int			uSynergyBool;
#define				USYNERGY_FALSE					0
#define				USYNERGY_TRUE					1


typedef struct { int ignored; } *					uSynergyCookie;


enum uSynergyClipboardFormat
{
	USYNERGY_CLIPBOARD_FORMAT_TEXT					= 0,
	USYNERGY_CLIPBOARD_FORMAT_BITMAP				= 1,
	USYNERGY_CLIPBOARD_FORMAT_HTML					= 2,
};


#define				USYNERGY_NUM_JOYSTICKS			4

#define				USYNERGY_PROTOCOL_MAJOR			1
#define				USYNERGY_PROTOCOL_MINOR			4

#define				USYNERGY_IDLE_TIMEOUT			2000

#define				USYNERGY_TRACE_BUFFER_SIZE		1024
#define				USYNERGY_REPLY_BUFFER_SIZE		1024
#define				USYNERGY_RECEIVE_BUFFER_SIZE	4096


#define				USYNERGY_MODIFIER_SHIFT			0x0001
#define				USYNERGY_MODIFIER_CTRL			0x0002
#define				USYNERGY_MODIFIER_ALT			0x0004
#define				USYNERGY_MODIFIER_META			0x0008
#define				USYNERGY_MODIFIER_WIN			0x0010
#define				USYNERGY_MODIFIER_ALT_GR		0x0020
#define				USYNERGY_MODIFIER_LEVEL5LOCK	0x0040
#define				USYNERGY_MODIFIER_CAPSLOCK		0x1000
#define				USYNERGY_MODIFIER_NUMLOCK		0x2000
#define				USYNERGY_MODIFIER_SCROLLOCK		0x4000


typedef uSynergyBool (*uSynergyConnectFunc)(uSynergyCookie cookie);


typedef uSynergyBool (*uSynergySendFunc)(uSynergyCookie cookie, const uint8_t *buffer, int length);


typedef uSynergyBool (*uSynergyReceiveFunc)(uSynergyCookie cookie, uint8_t *buffer, int maxLength, int* outLength);


typedef void		(*uSynergySleepFunc)(uSynergyCookie cookie, int timeMs);


typedef uint32_t	(*uSynergyGetTimeFunc)();


typedef void		(*uSynergyTraceFunc)(uSynergyCookie cookie, const char *text);


typedef void		(*uSynergyScreenActiveCallback)(uSynergyCookie cookie, uSynergyBool active);


typedef void		(*uSynergyMouseCallback)(uSynergyCookie cookie, uint16_t x, uint16_t y, int16_t wheelX, int16_t wheelY, uSynergyBool buttonLeft, uSynergyBool buttonRight, uSynergyBool buttonMiddle);


typedef void		(*uSynergyKeyboardCallback)(uSynergyCookie cookie, uint16_t key, uint16_t modifiers, uSynergyBool down, uSynergyBool repeat);


typedef void		(*uSynergyJoystickCallback)(uSynergyCookie cookie, uint8_t joyNum, uint16_t buttons, int8_t leftStickX, int8_t leftStickY, int8_t rightStickX, int8_t rightStickY);


typedef void		(*uSynergyClipboardCallback)(uSynergyCookie cookie, enum uSynergyClipboardFormat format, const uint8_t *data, uint32_t size);


typedef struct
{

	uSynergyConnectFunc				m_connectFunc;
	uSynergySendFunc				m_sendFunc;
	uSynergyReceiveFunc				m_receiveFunc;
	uSynergySleepFunc				m_sleepFunc;
	uSynergyGetTimeFunc				m_getTimeFunc;
	const char*						m_clientName;
	uint16_t						m_clientWidth;
	uint16_t						m_clientHeight;


	uSynergyCookie					m_cookie;
	uSynergyTraceFunc				m_traceFunc;
	uSynergyScreenActiveCallback	m_screenActiveCallback;
	uSynergyMouseCallback			m_mouseCallback;
	uSynergyKeyboardCallback		m_keyboardCallback;
	uSynergyJoystickCallback		m_joystickCallback;
	uSynergyClipboardCallback		m_clipboardCallback;


	uSynergyBool					m_connected;
	uSynergyBool					m_hasReceivedHello;
	uSynergyBool					m_isCaptured;
	uint32_t						m_lastMessageTime;
	uint32_t						m_sequenceNumber;
	uint8_t							m_receiveBuffer[USYNERGY_RECEIVE_BUFFER_SIZE];
	int								m_receiveOfs;
	uint8_t							m_replyBuffer[USYNERGY_REPLY_BUFFER_SIZE];
	uint8_t*						m_replyCur;
	uint16_t						m_mouseX;
	uint16_t						m_mouseY;
	int16_t							m_mouseWheelX;
	int16_t							m_mouseWheelY;
	uSynergyBool					m_mouseButtonLeft;
	uSynergyBool					m_mouseButtonRight;
	uSynergyBool					m_mouseButtonMiddle;
	int8_t							m_joystickSticks[USYNERGY_NUM_JOYSTICKS][4];
	uint16_t						m_joystickButtons[USYNERGY_NUM_JOYSTICKS];
} uSynergyContext;


extern void		uSynergyInit(uSynergyContext *context);


extern void		uSynergyUpdate(uSynergyContext *context);


extern void		uSynergySendClipboard(uSynergyContext *context, const char *text);


#ifdef __cplusplus
};
#endif
