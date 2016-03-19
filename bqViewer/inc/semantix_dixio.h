/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2016  Mundoreader, S.L

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the source code.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

/*!
 ******************************************************************************
 * \file    main.h
 * \author  Blit
 * \note
 ******************************************************************************
 */

#ifndef _DICT_H
#define	_DICT_H

#include <list>

typedef std::string String;

//--------------- Stub

namespace Semantix
{
	struct dictionaryInfo{

	};

	void initialize(String log_folder);
	void initialize(String log_folder, String dictionariesPath);
	void finalize();
	String getDetectedLanguage(String key, const String& word, const String& context);
	std::list<String> getDictionariesByLanguage(String language);
	std::list<String> getDictionariesByLanguage(String language, int definitions, int translations);
	String search(String key, String word, String context, String source);
	String search(String key, String word, String context, String source, int definitions, int translations);
	String search(String key, String word, String context);
	String search(String key, String word, String context, int definitions, int translations);
	String searchEx(String key, String word, String context, String bookPath="");
	String searchXml(String key, String word, String context, int maxResult=3);
	String searchXml(String key, String word, String context, int maxResult, int definitions, int translations);
	String searchXmlEx(String key, String word, String context, String bookPath="", int maxResult=3);
	String searchHtml(String key, String word, String context,int maxResult=3, String source="");
	String searchHtml(String key, String word, String context, int maxResult, String source, int definitions, int translations);
	String searchHtmlEx(String key, String word, String context, String bookPath="", int maxResult=3);
	String getSerializedResult(String word, String definition, String key);
	String getVersion(void);
	String getVersion(String key);
	String getAbout(String key);
	String getDictionaries();
	String getWorkingDictionaries();
	const std::string getWorkingDictionariesEx();
	String getDictionaries(int definitions, int translations);
	String getDefaultExtension();
	String getName(String sourceId);
	String getDictionaryDescription(String sourceId);
	bool checkDeviceId(String deviceId);
	bool isValidKey(String key);
	/**
	 * gets url and parameters to send to dixio server
	 */
	bool getActivationData(String key, String interfaceToken, String& callUrl, String& postParameters);
	/**
	 * sets dictionary Activation from dixio server call
	 */
	String setActivationData(String key, String activationResponse);
	void setupDictionaryConfiguration(String path);
	void resetDictionariesPath();
}

#endif
