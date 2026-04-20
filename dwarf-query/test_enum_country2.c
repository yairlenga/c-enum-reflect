#include "enum_desc.h"

#define ISO2(a,b) ( ((a) << 8) | (b))

enum iso2_country_code {
    ISO2_AD = ISO2('A','D'), /* Andorra */
    ISO2_AE = ISO2('A','E'), /* United Arab Emirates */
    ISO2_AF = ISO2('A','F'), /* Afghanistan */
    ISO2_AG = ISO2('A','G'), /* Antigua and Barbuda */
    ISO2_AI = ISO2('A','I'), /* Anguilla */
    ISO2_AL = ISO2('A','L'), /* Albania */
    ISO2_AM = ISO2('A','M'), /* Armenia */
    ISO2_AO = ISO2('A','O'), /* Angola */
    ISO2_AQ = ISO2('A','Q'), /* Antarctica */
    ISO2_AR = ISO2('A','R'), /* Argentina */
    ISO2_AS = ISO2('A','S'), /* American Samoa */
    ISO2_AT = ISO2('A','T'), /* Austria */
    ISO2_AU = ISO2('A','U'), /* Australia */
    ISO2_AW = ISO2('A','W'), /* Aruba */
    ISO2_AX = ISO2('A','X'), /* Åland Islands */
    ISO2_AZ = ISO2('A','Z'), /* Azerbaijan */

    ISO2_BA = ISO2('B','A'), /* Bosnia and Herzegovina */
    ISO2_BB = ISO2('B','B'), /* Barbados */
    ISO2_BD = ISO2('B','D'), /* Bangladesh */
    ISO2_BE = ISO2('B','E'), /* Belgium */
    ISO2_BF = ISO2('B','F'), /* Burkina Faso */
    ISO2_BG = ISO2('B','G'), /* Bulgaria */
    ISO2_BH = ISO2('B','H'), /* Bahrain */
    ISO2_BI = ISO2('B','I'), /* Burundi */
    ISO2_BJ = ISO2('B','J'), /* Benin */
    ISO2_BL = ISO2('B','L'), /* Saint Barthélemy */
    ISO2_BM = ISO2('B','M'), /* Bermuda */
    ISO2_BN = ISO2('B','N'), /* Brunei */
    ISO2_BO = ISO2('B','O'), /* Bolivia */
    ISO2_BQ = ISO2('B','Q'), /* Caribbean Netherlands */
    ISO2_BR = ISO2('B','R'), /* Brazil */
    ISO2_BS = ISO2('B','S'), /* Bahamas */
    ISO2_BT = ISO2('B','T'), /* Bhutan */
    ISO2_BV = ISO2('B','V'), /* Bouvet Island */
    ISO2_BW = ISO2('B','W'), /* Botswana */
    ISO2_BY = ISO2('B','Y'), /* Belarus */
    ISO2_BZ = ISO2('B','Z'), /* Belize */

    ISO2_CA = ISO2('C','A'), /* Canada */
    ISO2_CC = ISO2('C','C'), /* Cocos (Keeling) Islands */
    ISO2_CD = ISO2('C','D'), /* Congo, Democratic Republic */
    ISO2_CF = ISO2('C','F'), /* Central African Republic */
    ISO2_CG = ISO2('C','G'), /* Congo */
    ISO2_CH = ISO2('C','H'), /* Switzerland */
    ISO2_CI = ISO2('C','I'), /* Côte d'Ivoire */
    ISO2_CK = ISO2('C','K'), /* Cook Islands */
    ISO2_CL = ISO2('C','L'), /* Chile */
    ISO2_CM = ISO2('C','M'), /* Cameroon */
    ISO2_CN = ISO2('C','N'), /* China */
    ISO2_CO = ISO2('C','O'), /* Colombia */
    ISO2_CR = ISO2('C','R'), /* Costa Rica */
    ISO2_CU = ISO2('C','U'), /* Cuba */
    ISO2_CV = ISO2('C','V'), /* Cabo Verde */
    ISO2_CW = ISO2('C','W'), /* Curaçao */
    ISO2_CX = ISO2('C','X'), /* Christmas Island */
    ISO2_CY = ISO2('C','Y'), /* Cyprus */
    ISO2_CZ = ISO2('C','Z'), /* Czechia */

    ISO2_DE = ISO2('D','E'), /* Germany */
    ISO2_DJ = ISO2('D','J'), /* Djibouti */
    ISO2_DK = ISO2('D','K'), /* Denmark */
    ISO2_DM = ISO2('D','M'), /* Dominica */
    ISO2_DO = ISO2('D','O'), /* Dominican Republic */
    ISO2_DZ = ISO2('D','Z'), /* Algeria */

    ISO2_EC = ISO2('E','C'), /* Ecuador */
    ISO2_EE = ISO2('E','E'), /* Estonia */
    ISO2_EG = ISO2('E','G'), /* Egypt */
    ISO2_EH = ISO2('E','H'), /* Western Sahara */
    ISO2_ER = ISO2('E','R'), /* Eritrea */
    ISO2_ES = ISO2('E','S'), /* Spain */
    ISO2_ET = ISO2('E','T'), /* Ethiopia */

    ISO2_FI = ISO2('F','I'), /* Finland */
    ISO2_FJ = ISO2('F','J'), /* Fiji */
    ISO2_FK = ISO2('F','K'), /* Falkland Islands */
    ISO2_FM = ISO2('F','M'), /* Micronesia */
    ISO2_FO = ISO2('F','O'), /* Faroe Islands */
    ISO2_FR = ISO2('F','R'), /* France */

    ISO2_GA = ISO2('G','A'), /* Gabon */
    ISO2_GB = ISO2('G','B'), /* United Kingdom */
    ISO2_GD = ISO2('G','D'), /* Grenada */
    ISO2_GE = ISO2('G','E'), /* Georgia */
    ISO2_GF = ISO2('G','F'), /* French Guiana */
    ISO2_GG = ISO2('G','G'), /* Guernsey */
    ISO2_GH = ISO2('G','H'), /* Ghana */
    ISO2_GI = ISO2('G','I'), /* Gibraltar */
    ISO2_GL = ISO2('G','L'), /* Greenland */
    ISO2_GM = ISO2('G','M'), /* Gambia */
    ISO2_GN = ISO2('G','N'), /* Guinea */
    ISO2_GP = ISO2('G','P'), /* Guadeloupe */
    ISO2_GQ = ISO2('G','Q'), /* Equatorial Guinea */
    ISO2_GR = ISO2('G','R'), /* Greece */
    ISO2_GS = ISO2('G','S'), /* South Georgia and the South Sandwich Islands */
    ISO2_GT = ISO2('G','T'), /* Guatemala */
    ISO2_GU = ISO2('G','U'), /* Guam */
    ISO2_GW = ISO2('G','W'), /* Guinea-Bissau */
    ISO2_GY = ISO2('G','Y'), /* Guyana */

    ISO2_HK = ISO2('H','K'), /* Hong Kong */
    ISO2_HM = ISO2('H','M'), /* Heard Island and McDonald Islands */
    ISO2_HN = ISO2('H','N'), /* Honduras */
    ISO2_HR = ISO2('H','R'), /* Croatia */
    ISO2_HT = ISO2('H','T'), /* Haiti */
    ISO2_HU = ISO2('H','U'), /* Hungary */

    ISO2_ID = ISO2('I','D'), /* Indonesia */
    ISO2_IE = ISO2('I','E'), /* Ireland */
    ISO2_IL = ISO2('I','L'), /* Israel */
    ISO2_IM = ISO2('I','M'), /* Isle of Man */
    ISO2_IN = ISO2('I','N'), /* India */
    ISO2_IO = ISO2('I','O'), /* British Indian Ocean Territory */
    ISO2_IQ = ISO2('I','Q'), /* Iraq */
    ISO2_IR = ISO2('I','R'), /* Iran */
    ISO2_IS = ISO2('I','S'), /* Iceland */
    ISO2_IT = ISO2('I','T'), /* Italy */

    ISO2_JE = ISO2('J','E'), /* Jersey */
    ISO2_JM = ISO2('J','M'), /* Jamaica */
    ISO2_JO = ISO2('J','O'), /* Jordan */
    ISO2_JP = ISO2('J','P'), /* Japan */

    ISO2_KE = ISO2('K','E'), /* Kenya */
    ISO2_KG = ISO2('K','G'), /* Kyrgyzstan */
    ISO2_KH = ISO2('K','H'), /* Cambodia */
    ISO2_KI = ISO2('K','I'), /* Kiribati */
    ISO2_KM = ISO2('K','M'), /* Comoros */
    ISO2_KN = ISO2('K','N'), /* Saint Kitts and Nevis */
    ISO2_KP = ISO2('K','P'), /* North Korea */
    ISO2_KR = ISO2('K','R'), /* South Korea */
    ISO2_KW = ISO2('K','W'), /* Kuwait */
    ISO2_KY = ISO2('K','Y'), /* Cayman Islands */
    ISO2_KZ = ISO2('K','Z'), /* Kazakhstan */

    ISO2_LA = ISO2('L','A'), /* Laos */
    ISO2_LB = ISO2('L','B'), /* Lebanon */
    ISO2_LC = ISO2('L','C'), /* Saint Lucia */
    ISO2_LI = ISO2('L','I'), /* Liechtenstein */
    ISO2_LK = ISO2('L','K'), /* Sri Lanka */
    ISO2_LR = ISO2('L','R'), /* Liberia */
    ISO2_LS = ISO2('L','S'), /* Lesotho */
    ISO2_LT = ISO2('L','T'), /* Lithuania */
    ISO2_LU = ISO2('L','U'), /* Luxembourg */
    ISO2_LV = ISO2('L','V'), /* Latvia */
    ISO2_LY = ISO2('L','Y'), /* Libya */

    ISO2_MA = ISO2('M','A'), /* Morocco */
    ISO2_MC = ISO2('M','C'), /* Monaco */
    ISO2_MD = ISO2('M','D'), /* Moldova */
    ISO2_ME = ISO2('M','E'), /* Montenegro */
    ISO2_MF = ISO2('M','F'), /* Saint Martin */
    ISO2_MG = ISO2('M','G'), /* Madagascar */
    ISO2_MH = ISO2('M','H'), /* Marshall Islands */
    ISO2_MK = ISO2('M','K'), /* North Macedonia */
    ISO2_ML = ISO2('M','L'), /* Mali */
    ISO2_MM = ISO2('M','M'), /* Myanmar */
    ISO2_MN = ISO2('M','N'), /* Mongolia */
    ISO2_MO = ISO2('M','O'), /* Macao */
    ISO2_MP = ISO2('M','P'), /* Northern Mariana Islands */
    ISO2_MQ = ISO2('M','Q'), /* Martinique */
    ISO2_MR = ISO2('M','R'), /* Mauritania */
    ISO2_MS = ISO2('M','S'), /* Montserrat */
    ISO2_MT = ISO2('M','T'), /* Malta */
    ISO2_MU = ISO2('M','U'), /* Mauritius */
    ISO2_MV = ISO2('M','V'), /* Maldives */
    ISO2_MW = ISO2('M','W'), /* Malawi */
    ISO2_MX = ISO2('M','X'), /* Mexico */
    ISO2_MY = ISO2('M','Y'), /* Malaysia */
    ISO2_MZ = ISO2('M','Z'), /* Mozambique */

    ISO2_NA = ISO2('N','A'), /* Namibia */
    ISO2_NC = ISO2('N','C'), /* New Caledonia */
    ISO2_NE = ISO2('N','E'), /* Niger */
    ISO2_NF = ISO2('N','F'), /* Norfolk Island */
    ISO2_NG = ISO2('N','G'), /* Nigeria */
    ISO2_NI = ISO2('N','I'), /* Nicaragua */
    ISO2_NL = ISO2('N','L'), /* Netherlands */
    ISO2_NO = ISO2('N','O'), /* Norway */
    ISO2_NP = ISO2('N','P'), /* Nepal */
    ISO2_NR = ISO2('N','R'), /* Nauru */
    ISO2_NU = ISO2('N','U'), /* Niue */
    ISO2_NZ = ISO2('N','Z'), /* New Zealand */

    ISO2_OM = ISO2('O','M'), /* Oman */

    ISO2_PA = ISO2('P','A'), /* Panama */
    ISO2_PE = ISO2('P','E'), /* Peru */
    ISO2_PF = ISO2('P','F'), /* French Polynesia */
    ISO2_PG = ISO2('P','G'), /* Papua New Guinea */
    ISO2_PH = ISO2('P','H'), /* Philippines */
    ISO2_PK = ISO2('P','K'), /* Pakistan */
    ISO2_PL = ISO2('P','L'), /* Poland */
    ISO2_PM = ISO2('P','M'), /* Saint Pierre and Miquelon */
    ISO2_PN = ISO2('P','N'), /* Pitcairn */
    ISO2_PR = ISO2('P','R'), /* Puerto Rico */
    ISO2_PS = ISO2('P','S'), /* Palestine */
    ISO2_PT = ISO2('P','T'), /* Portugal */
    ISO2_PW = ISO2('P','W'), /* Palau */
    ISO2_PY = ISO2('P','Y'), /* Paraguay */

    ISO2_QA = ISO2('Q','A'), /* Qatar */

    ISO2_RE = ISO2('R','E'), /* Réunion */
    ISO2_RO = ISO2('R','O'), /* Romania */
    ISO2_RS = ISO2('R','S'), /* Serbia */
    ISO2_RU = ISO2('R','U'), /* Russia */
    ISO2_RW = ISO2('R','W'), /* Rwanda */

    ISO2_SA = ISO2('S','A'), /* Saudi Arabia */
    ISO2_SB = ISO2('S','B'), /* Solomon Islands */
    ISO2_SC = ISO2('S','C'), /* Seychelles */
    ISO2_SD = ISO2('S','D'), /* Sudan */
    ISO2_SE = ISO2('S','E'), /* Sweden */
    ISO2_SG = ISO2('S','G'), /* Singapore */
    ISO2_SH = ISO2('S','H'), /* Saint Helena */
    ISO2_SI = ISO2('S','I'), /* Slovenia */
    ISO2_SJ = ISO2('S','J'), /* Svalbard and Jan Mayen */
    ISO2_SK = ISO2('S','K'), /* Slovakia */
    ISO2_SL = ISO2('S','L'), /* Sierra Leone */
    ISO2_SM = ISO2('S','M'), /* San Marino */
    ISO2_SN = ISO2('S','N'), /* Senegal */
    ISO2_SO = ISO2('S','O'), /* Somalia */
    ISO2_SR = ISO2('S','R'), /* Suriname */
    ISO2_SS = ISO2('S','S'), /* South Sudan */
    ISO2_ST = ISO2('S','T'), /* São Tomé and Príncipe */
    ISO2_SV = ISO2('S','V'), /* El Salvador */
    ISO2_SX = ISO2('S','X'), /* Sint Maarten */
    ISO2_SY = ISO2('S','Y'), /* Syria */
    ISO2_SZ = ISO2('S','Z'), /* Eswatini */

    ISO2_TC = ISO2('T','C'), /* Turks and Caicos Islands */
    ISO2_TD = ISO2('T','D'), /* Chad */
    ISO2_TF = ISO2('T','F'), /* French Southern Territories */
    ISO2_TG = ISO2('T','G'), /* Togo */
    ISO2_TH = ISO2('T','H'), /* Thailand */
    ISO2_TJ = ISO2('T','J'), /* Tajikistan */
    ISO2_TK = ISO2('T','K'), /* Tokelau */
    ISO2_TL = ISO2('T','L'), /* Timor-Leste */
    ISO2_TM = ISO2('T','M'), /* Turkmenistan */
    ISO2_TN = ISO2('T','N'), /* Tunisia */
    ISO2_TO = ISO2('T','O'), /* Tonga */
    ISO2_TR = ISO2('T','R'), /* Türkiye */
    ISO2_TT = ISO2('T','T'), /* Trinidad and Tobago */
    ISO2_TV = ISO2('T','V'), /* Tuvalu */
    ISO2_TW = ISO2('T','W'), /* Taiwan */
    ISO2_TZ = ISO2('T','Z'), /* Tanzania */

    ISO2_UA = ISO2('U','A'), /* Ukraine */
    ISO2_UG = ISO2('U','G'), /* Uganda */
    ISO2_UM = ISO2('U','M'), /* U.S. Minor Outlying Islands */
    ISO2_US = ISO2('U','S'), /* United States */
    ISO2_UY = ISO2('U','Y'), /* Uruguay */
    ISO2_UZ = ISO2('U','Z'), /* Uzbekistan */

    ISO2_VA = ISO2('V','A'), /* Vatican City */
    ISO2_VC = ISO2('V','C'), /* Saint Vincent and the Grenadines */
    ISO2_VE = ISO2('V','E'), /* Venezuela */
    ISO2_VG = ISO2('V','G'), /* British Virgin Islands */
    ISO2_VI = ISO2('V','I'), /* U.S. Virgin Islands */
    ISO2_VN = ISO2('V','N'), /* Vietnam */
    ISO2_VU = ISO2('V','U'), /* Vanuatu */

    ISO2_WF = ISO2('W','F'), /* Wallis and Futuna */
    ISO2_WS = ISO2('W','S'), /* Samoa */

    ISO2_YE = ISO2('Y','E'), /* Yemen */
    ISO2_YT = ISO2('Y','T'), /* Mayotte */

    ISO2_ZA = ISO2('Z','A'), /* South Africa */
    ISO2_ZM = ISO2('Z','M'), /* Zambia */
    ISO2_ZW = ISO2('Z','W')  /* Zimbabwe */
};

ENUM_DESCRIBE(country2, enum iso2_country_code)
