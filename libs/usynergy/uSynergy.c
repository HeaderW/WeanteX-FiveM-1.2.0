


#include "uSynergy.h"
#include <stdio.h>
#include <string.h>


static int16_t sNetToNative16(const unsigned char *value)
{
#ifdef USYNERGY_LITTLE_ENDIAN
	return value[1] | (value[0] << 8);
#else
	return value[0] | (value[1] << 8);
#endif
}


static int32_t sNetToNative32(const unsigned char *value)
{
#ifdef USYNERGY_LITTLE_ENDIAN
	return value[3] | (value[2] << 8) | (value[1] << 16) | (value[0] << 24);
#else
	return value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
#endif
}


static void sTrace(uSynergyContext *context, const char* text)
{

	if (context->m_traceFunc != 0L)
		context->m_traceFunc(context->m_cookie, text);
}


static void sAddString(uSynergyContext *context, const char *string)
{
	size_t len = strlen(string);
	memcpy(context->m_replyCur, string, len);
	context->m_replyCur += len;
}


static void sAddUInt8(uSynergyContext *context, uint8_t value)
{
	*context->m_replyCur++ = value;
}


static void sAddUInt16(uSynergyContext *context, uint16_t value)
{
	uint8_t *reply = context->m_replyCur;
	*reply++ = (uint8_t)(value >> 8);
	*reply++ = (uint8_t)value;
	context->m_replyCur = reply;
}


static void sAddUInt32(uSynergyContext *context, uint32_t value)
{
	uint8_t *reply = context->m_replyCur;
	*reply++ = (uint8_t)(value >> 24);
	*reply++ = (uint8_t)(value >> 16);
	*reply++ = (uint8_t)(value >> 8);
	*reply++ = (uint8_t)value;
	context->m_replyCur = reply;
}


static uSynergyBool sSendReply(uSynergyContext *context)
{

	uint8_t		*reply_buf	= context->m_replyBuffer;
	uint32_t	reply_len	= (uint32_t)(context->m_replyCur - reply_buf);
	uint32_t	body_len	= reply_len - 4;
	uSynergyBool ret;
	reply_buf[0] = (uint8_t)(body_len >> 24);
	reply_buf[1] = (uint8_t)(body_len >> 16);
	reply_buf[2] = (uint8_t)(body_len >> 8);
	reply_buf[3] = (uint8_t)body_len;


	ret = context->m_sendFunc(context->m_cookie, context->m_replyBuffer, reply_len);


	context->m_replyCur = context->m_replyBuffer+4;
	return ret;
}


static void sSendMouseCallback(uSynergyContext *context)
{

	if (context->m_mouseCallback == 0L)
		return;


	context->m_mouseCallback(context->m_cookie, context->m_mouseX, context->m_mouseY, context->m_mouseWheelX,
		context->m_mouseWheelY, context->m_mouseButtonLeft, context->m_mouseButtonRight, context->m_mouseButtonMiddle);
}


static void sSendKeyboardCallback(uSynergyContext *context, uint16_t key, uint16_t modifiers, uSynergyBool down, uSynergyBool repeat)
{

	if (context->m_keyboardCallback == 0L)
		return;


	context->m_keyboardCallback(context->m_cookie, key, modifiers, down, repeat);
}


static void sSendJoystickCallback(uSynergyContext *context, uint8_t joyNum)
{
	int8_t *sticks;


	if (context->m_joystickCallback == 0L)
		return;


	sticks = context->m_joystickSticks[joyNum];
	context->m_joystickCallback(context->m_cookie, joyNum, context->m_joystickButtons[joyNum], sticks[0], sticks[1], sticks[2], sticks[3]);
}


#define USYNERGY_IS_PACKET(pkt_id)	memcmp(message+4, pkt_id, 4)==0
static void sProcessMessage(uSynergyContext *context, const uint8_t *message)
{

	if (memcmp(message+4, "Synergy", 7)==0)
	{


		sAddString(context, "Synergy");
		sAddUInt16(context, USYNERGY_PROTOCOL_MAJOR);
		sAddUInt16(context, USYNERGY_PROTOCOL_MINOR);
		sAddUInt32(context, (uint32_t)strlen(context->m_clientName));
		sAddString(context, context->m_clientName);
		if (!sSendReply(context))
		{

			sTrace(context, "SendReply failed, trying to reconnect in a second");
			context->m_connected = USYNERGY_FALSE;
			context->m_sleepFunc(context->m_cookie, 1000);
		}
		else
		{

			char buffer[256+1];
			sprintf(buffer, "Connected as client \"%s\"", context->m_clientName);
			sTrace(context, buffer);
			context->m_hasReceivedHello = USYNERGY_TRUE;
		}
		return;
	}
	else if (USYNERGY_IS_PACKET("QINF"))
	{


		uint16_t x = 0, y = 0, warp = 0;
		sAddString(context, "DINF");
		sAddUInt16(context, x);
		sAddUInt16(context, y);
		sAddUInt16(context, context->m_clientWidth);
		sAddUInt16(context, context->m_clientHeight);
		sAddUInt16(context, warp);
		sAddUInt16(context, 0);
		sAddUInt16(context, 0);
		sSendReply(context);
		return;
	}
	else if (USYNERGY_IS_PACKET("CIAK"))
	{


		return;
	}
	else if (USYNERGY_IS_PACKET("CROP"))
	{


		return;
	}
	else if (USYNERGY_IS_PACKET("CINN"))
	{


		context->m_sequenceNumber = sNetToNative32(message + 12);
		context->m_isCaptured = USYNERGY_TRUE;


		if (context->m_screenActiveCallback != 0L)
			context->m_screenActiveCallback(context->m_cookie, USYNERGY_TRUE);
	}
	else if (USYNERGY_IS_PACKET("COUT"))
	{


		context->m_isCaptured = USYNERGY_FALSE;


		if (context->m_screenActiveCallback != 0L)
			context->m_screenActiveCallback(context->m_cookie, USYNERGY_FALSE);
	}
	else if (USYNERGY_IS_PACKET("DMDN"))
	{


		char btn = message[8]-1;
		if (btn==2)
			context->m_mouseButtonRight		= USYNERGY_TRUE;
		else if (btn==1)
			context->m_mouseButtonMiddle	= USYNERGY_TRUE;
		else
			context->m_mouseButtonLeft		= USYNERGY_TRUE;
		sSendMouseCallback(context);
	}
	else if (USYNERGY_IS_PACKET("DMUP"))
	{


		char btn = message[8]-1;
		if (btn==2)
			context->m_mouseButtonRight		= USYNERGY_FALSE;
		else if (btn==1)
			context->m_mouseButtonMiddle	= USYNERGY_FALSE;
		else
			context->m_mouseButtonLeft		= USYNERGY_FALSE;
		sSendMouseCallback(context);
	}
	else if (USYNERGY_IS_PACKET("DMMV"))
	{


		context->m_mouseX = sNetToNative16(message+8);
		context->m_mouseY = sNetToNative16(message+10);
		sSendMouseCallback(context);
	}
	else if (USYNERGY_IS_PACKET("DMWM"))
	{


		context->m_mouseWheelX += sNetToNative16(message+8);
		context->m_mouseWheelY += sNetToNative16(message+10);
		sSendMouseCallback(context);
	}
	else if (USYNERGY_IS_PACKET("DKDN"))
	{


		uint16_t mod = sNetToNative16(message+10);
		uint16_t key = sNetToNative16(message+12);
		sSendKeyboardCallback(context, key, mod, USYNERGY_TRUE, USYNERGY_FALSE);
	}
	else if (USYNERGY_IS_PACKET("DKRP"))
	{


		uint16_t mod = sNetToNative16(message+10);

		uint16_t key = sNetToNative16(message+14);
		sSendKeyboardCallback(context, key, mod, USYNERGY_TRUE, USYNERGY_TRUE);
	}
	else if (USYNERGY_IS_PACKET("DKUP"))
	{


		uint16_t mod = sNetToNative16(message+10);
		uint16_t key = sNetToNative16(message+12);
		sSendKeyboardCallback(context, key, mod, USYNERGY_FALSE, USYNERGY_FALSE);
	}
	else if (USYNERGY_IS_PACKET("DGBT"))
	{


		uint8_t	joy_num = message[8];
		if (joy_num<USYNERGY_NUM_JOYSTICKS)
		{

			context->m_joystickButtons[joy_num] = (message[9] << 8) | message[10];
			sSendJoystickCallback(context, joy_num);
		}
	}
	else if (USYNERGY_IS_PACKET("DGST"))
	{


		uint8_t	joy_num = message[8];
		if (joy_num<USYNERGY_NUM_JOYSTICKS)
		{

			memcpy(context->m_joystickSticks[joy_num], message+9, 4);
			sSendJoystickCallback(context, joy_num);
		}
	}
	else if (USYNERGY_IS_PACKET("DSOP"))
	{


	}
	else if (USYNERGY_IS_PACKET("CALV"))
	{


		sAddString(context, "CALV");
		sSendReply(context);

	}
	else if (USYNERGY_IS_PACKET("DCLP"))
	{


		const uint8_t *	parse_msg	= message+17;
		uint32_t		num_formats = sNetToNative32(parse_msg);
		parse_msg += 4;
		for (; num_formats; num_formats--)
		{

			uint32_t format	= sNetToNative32(parse_msg);
			uint32_t size	= sNetToNative32(parse_msg+4);
			parse_msg += 8;


			if (context->m_clipboardCallback)
				context->m_clipboardCallback(context->m_cookie, format, parse_msg, size);

			parse_msg += size;
		}
	}
	else
	{


		char buffer[64];
		sprintf(buffer, "Unknown packet '%c%c%c%c'", message[4], message[5], message[6], message[7]);
		sTrace(context, buffer);
		return;
	}


	sAddString(context, "CNOP");
	sSendReply(context);
}
#undef USYNERGY_IS_PACKET


static void sSetDisconnected(uSynergyContext *context)
{
	context->m_connected		= USYNERGY_FALSE;
	context->m_hasReceivedHello = USYNERGY_FALSE;
	context->m_isCaptured		= USYNERGY_FALSE;
	context->m_replyCur			= context->m_replyBuffer + 4;
	context->m_sequenceNumber	= 0;
}


static void sUpdateContext(uSynergyContext *context)
{

	int receive_size = USYNERGY_RECEIVE_BUFFER_SIZE - context->m_receiveOfs;
	int num_received = 0;
	int packlen = 0;
	if (context->m_receiveFunc(context->m_cookie, context->m_receiveBuffer + context->m_receiveOfs, receive_size, &num_received) == USYNERGY_FALSE)
	{

		char buffer[128];
		sprintf(buffer, "Receive failed (%d bytes asked, %d bytes received), trying to reconnect in a second", receive_size, num_received);
		sTrace(context, buffer);
		sSetDisconnected(context);
		context->m_sleepFunc(context->m_cookie, 1000);
		return;
	}
	context->m_receiveOfs += num_received;


	if (num_received == 0)
		context->m_sleepFunc(context->m_cookie, 500);


	if (context->m_hasReceivedHello)
	{
		uint32_t cur_time = context->m_getTimeFunc();
		if (num_received == 0)
		{

			if ((cur_time - context->m_lastMessageTime) > USYNERGY_IDLE_TIMEOUT)
				sSetDisconnected(context);
		}
		else
			context->m_lastMessageTime = cur_time;
	}


	for (;;)
	{

		packlen = sNetToNative32(context->m_receiveBuffer);
		if (packlen+4 > context->m_receiveOfs)
			break;


		sProcessMessage(context, context->m_receiveBuffer);


		memmove(context->m_receiveBuffer, context->m_receiveBuffer+packlen+4, context->m_receiveOfs-packlen-4);
		context->m_receiveOfs -= packlen+4;
	}


	if (packlen > USYNERGY_RECEIVE_BUFFER_SIZE)
	{

		char buffer[128];
		sprintf(buffer, "Oversized packet: '%c%c%c%c' (length %d)", context->m_receiveBuffer[4], context->m_receiveBuffer[5], context->m_receiveBuffer[6], context->m_receiveBuffer[7], packlen);
		sTrace(context, buffer);
		num_received = context->m_receiveOfs-4;
		while (num_received != packlen)
		{
			int buffer_left = packlen - num_received;
			int to_receive = buffer_left < USYNERGY_RECEIVE_BUFFER_SIZE ? buffer_left : USYNERGY_RECEIVE_BUFFER_SIZE;
			int ditch_received = 0;
			if (context->m_receiveFunc(context->m_cookie, context->m_receiveBuffer, to_receive, &ditch_received) == USYNERGY_FALSE)
			{

				sTrace(context, "Receive failed, trying to reconnect in a second");
				sSetDisconnected(context);
				context->m_sleepFunc(context->m_cookie, 1000);
				break;
			}
			else
			{
				num_received += ditch_received;
			}
		}
		context->m_receiveOfs = 0;
	}
}


void uSynergyInit(uSynergyContext *context)
{

	memset(context, 0, sizeof(uSynergyContext));


	sSetDisconnected(context);
}


void uSynergyUpdate(uSynergyContext *context)
{
	if (context->m_connected)
	{

		sUpdateContext(context);
	}
	else
	{

		if (context->m_connectFunc(context->m_cookie))
			context->m_connected = USYNERGY_TRUE;
	}
}


void uSynergySendClipboard(uSynergyContext *context, const char *text)
{

	uint32_t overhead_size =	4 +
								4 +
								1 +
								4 +
								4 +
								4 +
								4 +
								4;
	uint32_t max_length = USYNERGY_REPLY_BUFFER_SIZE - overhead_size;


	uint32_t text_length = (uint32_t)strlen(text);
	if (text_length > max_length)
	{
		char buffer[128];
		sprintf(buffer, "Clipboard buffer too small, clipboard truncated at %d characters", max_length);
		sTrace(context, buffer);
		text_length = max_length;
	}


	sAddString(context, "DCLP");
	sAddUInt8(context, 0);
	sAddUInt32(context, context->m_sequenceNumber);
	sAddUInt32(context, 4+4+4+text_length);
	sAddUInt32(context, 1);
	sAddUInt32(context, USYNERGY_CLIPBOARD_FORMAT_TEXT);
	sAddUInt32(context, text_length);
	sAddString(context, text);
	sSendReply(context);
}
