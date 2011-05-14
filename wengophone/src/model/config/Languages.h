/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef LANGUAGES_H
#define LANGUAGES_H

/** A language is the association of its ISO 639 code + its name + its english name. */
struct Language {

	/** ISO 639 Alpha-2 code. */
	std::string iso639Code;

	/** Original language name. */
	std::string name;

	/** English language name. */
	std::string englishName;
};


/**
 * List of all languages.
 *
 * Taken from wikipedia.
 *
 * Format:
 * ISO 639 Alpha-2 code
 * language name
 * language name in english
 *
 * Originaly created on 31th october 2005 from revision 1.53 of Names.php
 *
 * @see http://cvs.sourceforge.net/viewcvs.py/wikipedia/phase3/languages/Names.php
 * @see http://en.wikipedia.org/w/index.php?title=ISO_639
 * @author Tanguy Krotoff
 */
static const Language LANGUAGES[] = {
	{ "aa", "Afar", "Afar" },
	{ "ab", "Аҧсуа", "Abkhazian" },
	{ "af", "Afrikaans", "Afrikaans" },
	{ "ak", "Akana", "Akan" },
	{ "als", "Alemannisch", "Alemannic" },
	{ "am", "አማርኛ", "Amharic" },
	{ "an", "Aragonés", "Aragonese" },
	{ "ang", "Anglo Saxon", "Old English" },
	{ "ar", "العربية", "Arabic" },
	{ "arc", "ܕܥܒܪܸܝܛ", "Aramaic" },
	{ "as", "অসমীয়া", "Assamese" },
	{ "ast", "Asturianu", "Asturian" },
	{ "av", "Авар", "Avar" },
	{ "ay", "Aymar", "Aymara" },
	{ "az", "Azərbaycan", "Azerbaijani" },
	{ "ba", "Башҡорт", "Bashkir" },
	{ "bat-smg", "Žemaitėška", "Samogitian" },
	{ "be", "Беларуская", "Byelarussian" },
	{ "bg", "Български", "Bulgarian" },
	{ "bh", "भोजपुरी", "Bihara" },
	{ "bi", "Bislama", "Bislama" },
	{ "bm", "Bamanankan", "Bambara" },
	{ "bn", "বাংলা", "Bengali" },
	{ "bo", "བོད་ཡིག", "Tibetan" },
	{ "br", "Brezhoneg", "Breton" },
	{ "bs", "Bosanski", "Bosnian" },
	{ "bug", "ᨅᨔ ᨕᨘᨁᨗ", "Buginese" },
	{ "ca", "Català", "Catalan" },
	{ "ce", "Нохчийн", "Chechen" },
	{ "ceb", "Cebuano", "Cebuano" },
	{ "ch", "Chamoru", "Chamorro" },
	{ "cho", "Choctaw", "Choctaw" },
	{ "chr", "ᏣᎳᎩ", "Cherokee" },
	{ "chy", "Tsetsêhestâhese", "Cheyenne" },
	{ "co", "Corsu", "Corsican" },
	{ "cr", "Nehiyaw", "Cree" },
	{ "cs", "Česky", "Czech" },
	{ "csb", "Kaszëbsczi", "Cassubian" },
	{ "cv", "Чӑваш", "Chuvash" },
	{ "cy", "Cymraeg", "Welsh" },
	{ "da", "Dansk", "Danish" },
	{ "de", "Deutsch", "German" },
	{ "dk", "Dansk", "Unused code currently redirecting to Danish da is correct for the language" },
	{ "dv", "ދިވެހިބަސް", "Dhivehi" },
	{ "dz", "ཇོང་ཁ", "Bhutani" },
	{ "ee", "Ɛʋɛ", "Ewe" },
	{ "el", "Ελληνικά", "Greek" },
	{ "en", "English", "English" },
	{ "en-gb", "English (British)", "English (British)" },
	{ "eo", "Esperanto", "Esperanto" },
	{ "es", "Español", "Spanish" },
	{ "et", "Eesti", "Estonian" },
	{ "eu", "Euskara", "Basque" },
	{ "fa", "فارسی", "Persian" },
	{ "ff", "Fulfulde", "Fulah" },
	{ "fi", "Suomi", "Finnish" },
	{ "fiu-vro", "Võro", "Võro" },
	{ "fj", "Na Vosa Vakaviti", "Fijian" },
	{ "fo", "Føroyskt", "Faroese" },
	{ "fr", "Français", "French" },
	{ "frp", "Arpitan", "Franco-Provençal/Arpitan" },
	{ "fur", "Furlan", "Friulian" },
	{ "fy", "Frysk", "Frisian" },
	{ "ga", "Gaeilge", "Irish" },
	{ "gd", "Gàidhlig", "Scots Gaelic" },
	{ "gl", "Galego", "Gallegan" },
	{ "gn", "Avañe - ẽ", "Guarani" },
	{ "got", "ЌӰЌ࠰ЍŰЌڰЍİЌډ", "Gothic" },
	{ "gsw", "Alemannisch", "Alemannic" },
	{ "gu", "ગુજરાતી", "Gujarati" },
	{ "gv", "Gaelg", "Manx" },
	{ "ha", "هَوُسَ", "Hausa" },
	{ "haw", "Hawai`i", "Hawaiian" },
	{ "he", "עברית", "Hebrew" },
	{ "hi", "हिन्दी", "Hindi" },
	{ "ho", "Hiri Motu", "Hiri Motu" },
	{ "hr", "Hrvatski", "Croatian" },
	{ "ht", "Krèyol ayisyen", "Haitian common" },
	{ "hu", "Magyar", "Hungarian" },
	{ "hy", "Հայերեն", "Armenian" },
	{ "hz", "Otsiherero", "Herero" },
	{ "ia", "Interlingua", "Interlingua (IALA)" },
	{ "id", "Bahasa Indonesia", "Indonesian" },
	{ "ie", "Interlingue", "Interlingue (Occidental)" },
	{ "ig", "Igbo", "Igbo" },
	{ "ii", "ꆇꉙ", "Sichuan Yi" },
	{ "ik", "Iñupiak", "Inupiak" },
	{ "ilo", "Ilokano", "Ilokano" },
	{ "io", "Ido", "Ido" },
	{ "is", "Íslenska", "Icelandic" },
	{ "it", "Italiano", "Italian" },
	{ "iu", "ᐃᓄᒃᑎᑐᑦ", "Inuktitut" },
	{ "ja", "日本語", "Japanese" },
	{ "jbo", "Lojban", "Lojban" },
	{ "jv", "Basa Jawa", "Javanese" },
	{ "ka", "ქართული", "Georgian" },
	{ "kg", "Kongo", "Kongo (FIXME!) should probaly be KiKongo or KiKoongo" },
	{ "ki", "Gĩkũyũ", "Kikuyu correctness not guaranteed" },
	{ "kj", "Kuanyama", "Kuanyama (FIXME!)" },
	{ "kk", "қазақша", "Kazakh" },
	{ "kl", "Kalaallisut", "Greenlandic" },
	{ "km", "ភាសាខ្មែរ", "Cambodian" },
	{ "kn", "ಕನ್ನಡ", "Kannada" },
	{ "ko", "한국어", "Korean" },
	{ "kr", "Kanuri", "Kanuri (FIXME!)" },
	{ "ks", "कश्मीरी - (كشميري)", "Kashmiri" },
	{ "ku", "Kurdî", "Kurdish" },
	{ "kv", "Коми ", "Komi cyrillic is common script but also written in latin script" },
	{ "kw", "Kernewek", "Cornish" },
	{ "ky", "Kırgızca", "Kirghiz" },
	{ "la", "Latina", "Latin" },
	{ "lad", "Ladino", "Ladino" },
	{ "lb", "Lëtzebuergesch", "Luxemburguish" },
	{ "lg", "Luganda", "Ganda" },
	{ "li", "Limburgs", "Limburgian" },
	{ "lmo", "Lumbaart", "Lombard" },
	{ "ln", "Lingala", "Lingala" },
	{ "lo", "ລາວ", "Laotian" },
	{ "lt", "Lietuvių", "Lithuanian" },
	{ "lv", "Latviešu", "Latvian" },
	{ "mg", "Malagasy", "Malagasy" },
	{ "mh", "Ebon", "Marshallese" },
	{ "mi", "Māori", "Maori" },
	{ "minnan", "Bân-lâm-gú", "Min-nan (also zh-min-nan)" },
	{ "mk", "Македонски", "Macedonian" },
	{ "ml", "മലയാളം", "Malayalam" },
	{ "mn", "Монгол", "Mongoloian" },
	{ "mo", "Молдовеняскэ", "Moldovan" },
	{ "mr", "मराठी", "Marathi" },
	{ "ms", "Bahasa Melayu", "Malay" },
	{ "mt", "bil-Malti", "Maltese" },
	{ "mus", "Muscogee", "Creek should possibly be Muskogee" },
	{ "my", "Myanmasa", "Burmese" },
	{ "na", "Ekakairũ Naoero", "Nauruan" },
	{ "nah", "Nahuatl", "Nahuatl en:Wikipedia writes Nahuatlahtolli while another form is Náhuatl" },
	{ "nap", "Nnapulitano", "Neapolitan" },
	{ "nb", "Norsk (bokmål)", "Norwegian (Bokmal)" },
	{ "nds", "Plattdüütsch", "Low German or Low Saxon" },
	{ "ne", "नेपाली", "Nepali" },
	{ "ng", "Oshiwambo", "Ndonga" },
	{ "nl", "Nederlands", "Dutch" },
	{ "nn", "Norsk (nynorsk)", "Norwegian (Nynorsk)" },
	{ "no", "Norsk (bokmål)", "Norwegian" },
	{ "non", "Norrǿna", "Old Norse" },
	{ "nv", "Diné bizaad", "Navajo" },
	{ "ny", "Chi-Chewa", "Chichewa" },
	{ "oc", "Occitan", "Occitan" },
	{ "om", "Oromoo ", "Oromo" },
	{ "or", "ଓଡ଼ିଆ", "Oriya" },
	{ "os", "Иронау", "Ossetic" },
	{ "pa", "ਪੰਜਾਬੀ", "Punjabi" },
	{ "pam", "Pampangan", "Pampanga" },
	{ "pih", "Norfuk - Pitkern", "Norfuk - Pitcairn - Norfolk" },
	{ "pi", "पािऴ", "Pali" },
	{ "pl", "Polski", "Polish" },
	{ "ps", "پښتو", "Pashto" },
	{ "pt", "Português", "Portuguese" },
	{ "pt-br", "Português do Brasil", "Brazilian Portuguese" },
	{ "qu", "Runa Simi", "Quechua" },
	{ "rm", "Rumantsch", "Raeto-Romance" },
	{ "rn", "Kirundi", "Kirundi" },
	{ "ro", "Română", "Romanian" },
	{ "roa-rup", "Armâneashti", "Aromanian" },
	{ "ru", "Русский", "Russian" },
	{ "rw", "Kinyarwanda", "Kinyarwanda should possibly be Kinyarwandi" },
	{ "sa", "संस्कृत", "Sanskrit" },
	{ "sc", "Sardu", "Sardinian" },
	{ "scn", "Sicilianu", "Sicilian" },
	{ "sco", "Scots", "Scots" },
	{ "sd", "सिनधि", "Sindhi" },
	{ "se", "Sámegiella", "Northern Sami" },
	{ "sg", "Sängö", "Sango possible alternative is Sangho" },
	{ "sh", "Srpskohrvatski - Српскохрватски", "Serbocroatian" },
	{ "si", "සිංහල", "Sinhalese" },
	{ "simple", "Simple English", "Simple English" },
	{ "sk", "Slovenčina", "Slovak" },
	{ "sl", "Slovenščina", "Slovenian" },
	{ "sm", "Gagana Samoa", "Samoan" },
	{ "sn", "chiShona", "Shona" },
	{ "so", "Soomaaliga", "Somali" },
	{ "sq", "Shqip", "Albanian" },
	{ "sr", "Српски - Srpski", "Serbian" },
	{ "ss", "SiSwati", "Swati" },
	{ "st", "seSotho", "Southern Sotho" },
	{ "su", "Basa Sunda", "Sundanese" },
	{ "sv", "Svenska", "Swedish" },
	{ "sw", "Kiswahili", "Swahili" },
	{ "ta", "தமிழ்", "Tamil" },
	{ "te", "తెలుగు", "Telugu" },
	{ "tg", "Тоҷикӣ", "Tajik" },
	{ "th", "ไทย", "Thai" },
	{ "ti", "ትግርኛ", "Tigrinya" },
	{ "tk", "تركمن - Туркмен", "Turkmen" },
	{ "tl", "Tagalog", "Tagalog (Filipino)" },
	{ "tn", "Setswana", "Setswana" },
	{ "to", "faka-Tonga", "Tonga (Tonga Islands)" },
	{ "tokipona", "Toki Pona", "Toki Pona" },
	{ "tp", "Toki Pona", "Toki Pona - non-standard language code" },
	{ "tpi", "Tok Pisin", "Tok Pisin" },
	{ "tr", "Türkçe", "Turkish" },
	{ "ts", "Xitsonga", "Tsonga" },
	{ "tt", "Tatarça", "Tatar" },
	{ "tum", "chiTumbuka", "Tumbuka" },
	{ "tw", "Twi", "Twi (FIXME!)" },
	{ "ty", "Reo Mā`ohi", "Tahitian" },
	{ "udm", "Удмурт", "Udmurt" },
	{ "ug", "Oyghurque", "Uyghur" },
	{ "uk", "Українська", "Ukrainian" },
	{ "ur", "اردو", "Urdu" },
	{ "uz", "Ўзбек", "Uzbek" },
	{ "ve", "Tshivenda", "Venda" },
	{ "vec", "Vèneto", "Venetian" },
	{ "vi", "Tiếng Việt", "Vietnamese" },
	{ "vo", "Volapük", "Volapük" },
	{ "wa", "Walon", "Walloon" },
	{ "war", "Samar-Leyte Visayan", "Waray-Waray" },
	{ "wo", "Wollof", "Wolof" },
	{ "xh", "isiXhosa", "Xhosan" },
	{ "yi", "ייִדיש", "Yiddish" },
	{ "yo", "Yorùbá", "Yoruba" },
	{ "za", "(Cuengh)", "Zhuang" },
	{ "zh", "中文", "(Zhōng Wén) - Chinese" },
	{ "zh-cfr", "閩南語", "Chinese Min-nan alias (site is at minnan)" },
	{ "zh-cn", "中文(简体)", "Chinese Simplified" },
	{ "zh-hk", "中文(繁體)", "ChineseTraditional (Hong Kong)" },
	{ "zh-min-nan", "Bân-lâm-gú", "Chinese Min-nan" },
	{ "zh-sg", "中文(简体)", "Chinese Simplified (Singapore)" },
	{ "zh-tw", "中文(繁體)", "Chinese Traditional" },
	{ "zu", "isiZulu", "Zulu" }
};

static const int LANGUAGES_SIZE = sizeof (LANGUAGES) / sizeof (Language);

#endif	//LANGUAGES_H
