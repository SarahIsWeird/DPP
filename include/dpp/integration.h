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

#include <dpp/json_fwd.hpp>

namespace dpp {

/**
 * @brief Integration types
 */
enum integration_type {
	i_twitch,
	i_youtube,
	i_discord
};

/**
 * @brief Integration flags
 */
enum integration_flags {
	if_enabled =     0b00000001,
	if_syncing =     0b00000010,
	if_emoticons =   0b00000100,
	if_revoked =     0b00001000,
	if_expire_kick = 0b00010000,
};

/**
 * @brief An application that has been integrated
 */
struct integration_app {
	snowflake id;
	std::string name;
	std::string icon;
	std::string description;
	std::string summary;
	user* bot;
};

/** Represents an integration within a dpp::guild */
class integration : public managed {
public:
	/** Integration name */
	std::string name;
	/** Integration type */
	integration_type type;
	/** Integration flags from dpp::integration_flags */
	uint8_t flags;
	/** Role id */
	snowflake role_id;
	/** User id */
	snowflake user_id;
	/** Expiry grace period */
	uint32_t expire_grace_period;
	/** Sync time */
	time_t synced_at;
	/** Subscriber count */
	uint32_t subscriber_count;
	/* Account id */
	std::string account_id;
	/* Account name */
	std::string account_name;
	/* Integration application */
	integration_app app;

	/** Default constructor */
	integration();

	/** Default destructor */
	~integration();

	/** Read class values from json object
	 * @param j A json object to read from
	 * @return A reference to self
	 */
	integration& fill_from_json(nlohmann::json* j);

	/** Build a json string from this object.
	 * @return JSON string of the object
	 */
	std::string build_json() const;

	/** True if emoticons are enabled */
	bool emoticons_enabled() const;
	/** True if integration is enabled */
	bool is_enabled() const;
	/** True if is syncing */
	bool is_syncing() const;
	/** True if has been revoked */
	bool is_revoked() const;
	/** True if expiring kicks the user */
	bool expiry_kicks_user() const;
};

/** A group of integrations */
typedef std::unordered_map<snowflake, integration> integration_map;

};

