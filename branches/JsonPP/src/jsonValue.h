/*
 * JsonValue class header file
 * Author: Sun Junwen
 */
#ifndef _JSON_VALUE_H_
#define _JSON_VALUE_H_

#include <cstdlib>
#include <string>
#include <map>

using namespace std;

class JsonValue
{
public:
	typedef map<string, string> StrMap;
	typedef map<string, JsonValue> JsonMap;

	/*
	 * Constructors
	 * Default is string value
	 */
	explicit JsonValue(bool bStr = true)
		:bStr(bStr)
	{};
	explicit JsonValue(const string& strValue)
		:bStr(true), strValue(strValue)
	{};

	// Get map value
	JsonMap GetMapValue() const;
	// Set map value
	void SetMapValue(const JsonMap& map);
	// Get string value
	string GetStrValue() const;
	// Set string value
	void SetStrValue(const string& str);
	
	// Is string value or not
	inline bool IsStrValue() const
	{ return bStr; }
	// Set value mode, true is string, false is not string
	inline void SetMode(bool bStr)
	{ this->bStr = bStr; }

	// Put key-value pair into map value
	void Put(const string& key, const string& value);

	// Convert string value or map value to string
	string ToString() const;

private:
	bool bStr;
	string strValue;
	JsonValue::JsonMap mapValue;
};

#endif
