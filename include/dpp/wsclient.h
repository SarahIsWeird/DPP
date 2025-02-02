/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <dpp/sslclient.h>

namespace dpp {

/**
 * @brief Websocket connection status
 */
enum WSState {
	/** Sending/receiving HTTP headers prior to protocol switch */
	HTTP_HEADERS,
	/** Connected, upgraded and sending/receiving frames */
	CONNECTED
};

/**
 * @brief Low-level websocket opcodes for frames
 */
enum OpCode
{
        OP_CONTINUATION = 0x00,	/* Continuation */
        OP_TEXT = 0x01,		/* Text frame */
        OP_BINARY = 0x02,	/* Binary frame */
        OP_CLOSE = 0x08,	/* Close notification with close code */
        OP_PING = 0x09,		/* Low level ping */
        OP_PONG = 0x0a		/* Low level pong */
};

/**
 * @brief Implements a websocket client based on the SSL client
 */
class WSClient : public SSLClient
{
	/** Connection key used in the HTTP headers */
	std::string key;

	/** Current websocket state */
	WSState state;

	/** Path part of URL for websocket */
	std::string path;

	/** HTTP headers received on connecting/upgrading */
	std::map<std::string, std::string> HTTPHeaders;

	/** Parse headers for a websocket frame from the buffer.
	 * @param buffer The buffer to operate on. Will modify the string removing completed items from the head of the queue
	 */
	bool parseheader(std::string &buffer);

	/** Unpack a frame and pass completed frames up the stack.
	 * @param buffer The buffer to operate on. Gets modified to remove completed frames on the head of the buffer
	 * @param offset The offset to start at (reserved for future use)
	 * @param first True if is the first element (reserved for future use)
	 */
	bool unpack(std::string &buffer, uint32_t offset, bool first = true);

	/** Fill a header for outbound messages
	 * @param outbuf The raw frame to fill
	 * @param sendlength The size of the data to encapsulate
	 * @param OpCode the opcode to send in the header
	 */
	size_t FillHeader(unsigned char* outbuf, size_t sendlength, OpCode opcode);

	/** Handle ping and pong requests.
	 * @param ping True if this is a ping, false if it is a pong 
	 * @param payload The ping payload, to be returned as-is for a ping
	 */
	void HandlePingPong(bool ping, const std::string &payload);

protected:

	/** (Re)connect */
	virtual void Connect();

	/** Get websocket state
	 * @return websocket state
	 */
	WSState GetState();

public:

	/** Connect to a specific websocket server.
	 * @param hostname Hostname to connect to
	 * @param port Port to connect to
	 * @param urlpath The URL path components of the HTTP request to send
	 */
        WSClient(const std::string &hostname, const std::string &port = "443", const std::string &urlpath = "");

	/** Destructor */
        virtual ~WSClient();

	/**
	 * @brief Write to websocket. Encapsulates data in frames if the status is CONNECTED.
	 * 
	 * @param data The data to send.
	 */
        virtual void write(const std::string &data);

	/**
	 * @brief Processes incoming frames from the SSL socket input buffer.
	 * 
	 * @param buffer The buffer contents. Can modify this value removing the head elements when processed.
	 */
        virtual bool HandleBuffer(std::string &buffer);

	/**
	 * @brief Close websocket
	 */
        virtual void close();

	/**
	 * @brief Receives raw frame content only without headers
	 * 
	 * @param buffer The buffer contents
	 * @return True if the frame was successfully handled. False if no valid frame is in the buffer.
	 */
	virtual bool HandleFrame(const std::string &buffer);

	/**
	 * @brief Called upon error frame.
	 * 
	 * @param errorcode The error code from the websocket server
	 */
	virtual void Error(uint32_t errorcode);

	/** Fires every second from the underlying socket I/O loop, used for sending webscocket pings */
	virtual void OneSecondTimer();
};

};