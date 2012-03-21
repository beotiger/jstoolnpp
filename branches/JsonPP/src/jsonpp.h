/* jsonpp.h
   2012-3-11
   Version: 0.9

Copyright (c) 2012 SUN Junwen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _JSONPP_H_
#define _JSONPP_H_

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "strhelper.h"

class JsonValue;
class JsonUnsortedMap;

typedef std::map<std::string, std::string> StrMap;
typedef std::map<std::string, JsonValue> JsonMap;
typedef std::pair<std::string, JsonValue> JsonMapPair;
typedef std::vector<JsonValue> JsonVec;

/**
 * �� std::vector<JsonMapPair> �ļ򻯰��װ
 * ������������ std::map ����
 */
class JsonUnsortedMap
{
public:
	typedef std::vector<JsonMapPair> JsonMapPairVec;
	typedef JsonMapPairVec::iterator iterator;
	typedef JsonMapPairVec::const_iterator const_iterator;

	inline void push_back(const JsonMapPair& pair)
	{ m_vec.push_back(pair); }

	inline const_iterator begin() const
	{ return m_vec.begin(); }
	inline iterator begin()
	{ return m_vec.begin(); }

	inline const_iterator end() const
	{ return m_vec.end(); }
	inline iterator end()
	{ return m_vec.end(); }

	// ֻ���ṩ O(n) ������
	JsonValue& operator[](const std::string& key);

private:
	JsonMapPairVec m_vec;
};


class JsonValue
{
public:
	enum VALUE_TYPE
	{
		STRING_VALUE = 0x00,
		NUMBER_VALUE = 0x01,
		BOOL_VALUE = 0x02,
		REGULAR_VALUE = 0x03,
		ARRAY_VALUE = 0x10, 
		MAP_VALUE = 0x20
	};

	/*
	 * Constructors
	 * Default is string value
	 */
	explicit JsonValue(VALUE_TYPE type = STRING_VALUE)
		:valType(type)
	{};
	explicit JsonValue(const std::string& strValue)
		:valType(STRING_VALUE), strValue(strValue)
	{};

	// Get string value
	std::string GetStrValue() const;
	// Set string value
	void SetStrValue(const std::string& str);
	// Get array value
	JsonVec GetArrayValue() const;
	// Set array value
	void SetArrayValue(const JsonVec& jArray);
	// Get map value
	JsonUnsortedMap GetMapValue() const;
	// Set map value
	void SetMapValue(const JsonUnsortedMap& jMap);
	
	// Is string value or not
	inline VALUE_TYPE GetValueType() const
	{ return valType; }
	// Set value mode, true is string, false is not string
	inline void SetValueType(VALUE_TYPE valType)
	{ this->valType = valType; }

	// Put key-value pair into array value
	void ArrayPut(const JsonValue& value);
	// Put key-value pair into map value
	void MapPut(const std::string& key, const JsonValue& value);

	// Convert string value or map value to string
	std::string ToString(int nRecuLevel = 0) const;

private:
	VALUE_TYPE valType;
	std::string strValue;
	JsonUnsortedMap mapValue;
	JsonVec arrayValue;
};

#endif
