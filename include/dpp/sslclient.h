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
#include <functional>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <dpp/discord.h>

namespace dpp {

/**
 * @brief Implements a simple non-blocking SSL stream client.
 * 
 * Note that although the design is non-blocking the Run() method will
 * execute in an infinite loop until the socket disconnects. This is intended
 * to be run within a std::thread.
 */
class SSLClient
{
protected:
	/** Input buffer received from openssl */
	std::string buffer;

	/** Output buffer for sending to openssl */
	std::string obuffer;

	/** True if in nonblocking mode. The socket switches to nonblocking mode
	 * once ReadLoop is called.
	 */
	bool nonblocking;

	/** Raw file descriptor of connection */
	int sfd;

	/** OpenSSL session */
	SSL* ssl;

	/** OpenSSL context */
	SSL_CTX* ctx;

	/** SSL cipher in use */
	std::string cipher;

	/** For timers */
	time_t last_tick;

	/** Hostname connected to */
	std::string hostname;

	/** Port connected to */
	std::string port;

	/** Bytes out */
	uint64_t bytes_out;

	/** Bytes in */
	uint64_t bytes_in;

	/** Called every second */
	virtual void OneSecondTimer();

	/** Start connection */
	virtual void Connect();
public:
	/** Get total bytes sent */
	uint64_t GetBytesOut();
	
	/** Get total bytes received */
	uint64_t GetBytesIn();

	/**
	 * @brief Attaching an additional file descriptor to this function will send notifications when there is data to read.
	 * 
	 * NOTE: Only hook this if you NEED it as it can increase CPU usage of the thread!
	 * Returning -1 means that you don't want to be notified.
	 */
	std::function<int()> custom_readable_fd;

	/**
	 * @brief Attaching an additional file descriptor to this function will send notifications when you are able to write
	 * to the socket.
	 * 
	 * NOTE: Only hook this if you NEED it as it can increase CPU usage of the thread! You should toggle this
	 * to -1 when you do not have anything to write otherwise it'll keep triggering repeatedly (it is level triggered).
	 */
	std::function<int()> custom_writeable_fd;

	/**
	 * @brief This event will be called when you can read from the custom fd
	 */
	std::function<void()> custom_readable_ready;

	/**
	 * @brief This event will be called when you can write to a custom fd
	 */
	std::function<void()> custom_writeable_ready;

	/**
	 * @brief Connect to a specified host and port. Throws std::runtime_error on fatal error.
	 * @param _hostname The hostname to connect to
	 * @param _port the Port number to connect to
	 */
	SSLClient(const std::string &_hostname, const std::string &_port = "443");

	/**
	 * @brief Nonblocking I/O loop
	 */
	void ReadLoop();

	/**
	 * @brief Destroy the SSLClient object
	 */
	virtual ~SSLClient();

	/**
	 * @brief Handle input from the input buffer.
	 * @param buffer the buffer content. Will be modified removing any processed front elements
	 */
	virtual bool HandleBuffer(std::string &buffer);

	/**
	 * @brief Write to the output buffer.
	 * @param data Data to be written to the buffer
	 */
	virtual void write(const std::string &data);

	/**
	 * @brief Close SSL connection
	 */
	virtual void close();

	/**
	 * @brief Log a message
	 * 
	 * @param severity severity of log message
	 * @param msg Log message to send
	 */
	virtual void log(dpp::loglevel severity, const std::string &msg);
};

};