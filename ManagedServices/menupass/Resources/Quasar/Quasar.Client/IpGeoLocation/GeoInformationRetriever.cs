using Quasar.Client.Helper;
using System.Globalization;
using System.IO;
using System.Net;

namespace Quasar.Client.IpGeoLocation
{
    /// <summary>
    /// Class to retrieve the IP geolocation information.
    /// </summary>
    public class GeoInformationRetriever
    {
        /// <summary>
        /// List of all available flag images on the server side.
        /// </summary>
        private readonly string[] _imageList =
        {
            "ad", "ae", "af", "ag", "ai", "al",
            "am", "an", "ao", "ar", "as", "at", "au", "aw", "ax", "az", "ba",
            "bb", "bd", "be", "bf", "bg", "bh", "bi", "bj", "bm", "bn", "bo",
            "br", "bs", "bt", "bv", "bw", "by", "bz", "ca", "catalonia", "cc",
            "cd", "cf", "cg", "ch", "ci", "ck", "cl", "cm", "cn", "co", "cr",
            "cs", "cu", "cv", "cx", "cy", "cz", "de", "dj", "dk", "dm", "do",
            "dz", "ec", "ee", "eg", "eh", "england", "er", "es", "et",
            "europeanunion", "fam", "fi", "fj", "fk", "fm", "fo", "fr", "ga",
            "gb", "gd", "ge", "gf", "gh", "gi", "gl", "gm", "gn", "gp", "gq",
            "gr", "gs", "gt", "gu", "gw", "gy", "hk", "hm", "hn", "hr", "ht",
            "hu", "id", "ie", "il", "in", "io", "iq", "ir", "is", "it", "jm",
            "jo", "jp", "ke", "kg", "kh", "ki", "km", "kn", "kp", "kr", "kw",
            "ky", "kz", "la", "lb", "lc", "li", "lk", "lr", "ls", "lt", "lu",
            "lv", "ly", "ma", "mc", "md", "me", "mg", "mh", "mk", "ml", "mm",
            "mn", "mo", "mp", "mq", "mr", "ms", "mt", "mu", "mv", "mw", "mx",
            "my", "mz", "na", "nc", "ne", "nf", "ng", "ni", "nl", "no", "np",
            "nr", "nu", "nz", "om", "pa", "pe", "pf", "pg", "ph", "pk", "pl",
            "pm", "pn", "pr", "ps", "pt", "pw", "py", "qa", "re", "ro", "rs",
            "ru", "rw", "sa", "sb", "sc", "scotland", "sd", "se", "sg", "sh",
            "si", "sj", "sk", "sl", "sm", "sn", "so", "sr", "st", "sv", "sy",
            "sz", "tc", "td", "tf", "tg", "th", "tj", "tk", "tl", "tm", "tn",
            "to", "tr", "tt", "tv", "tw", "tz", "ua", "ug", "um", "us", "uy",
            "uz", "va", "vc", "ve", "vg", "vi", "vn", "vu", "wales", "wf",
            "ws", "ye", "yt", "za", "zm", "zw"
        };

        /// <summary>
        /// Retrieves the IP geolocation information.
        /// </summary>
        /// <returns>The retrieved IP geolocation information.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1614: System Location Discovery
        /// </remarks>
        public GeoInformation Retrieve()
        {
            var geo = TryRetrieveLocally();

            geo.IpAddress = (string.IsNullOrEmpty(geo.IpAddress)) ? "Unknown" : geo.IpAddress;
            geo.Country = (string.IsNullOrEmpty(geo.Country)) ? "Unknown" : geo.Country;
            geo.CountryCode = (string.IsNullOrEmpty(geo.CountryCode)) ? "-" : geo.CountryCode;
            geo.Timezone = (string.IsNullOrEmpty(geo.Timezone)) ? "Unknown" : geo.Timezone;
            geo.Asn = (string.IsNullOrEmpty(geo.Asn)) ? "Unknown" : geo.Asn;
            geo.Isp = (string.IsNullOrEmpty(geo.Isp)) ? "Unknown" : geo.Isp;

            geo.ImageIndex = 0;
            for (int i = 0; i < _imageList.Length; i++)
            {
                if (_imageList[i] == geo.CountryCode.ToLower())
                {
                    geo.ImageIndex = i;
                    break;
                }
            }
            if (geo.ImageIndex == 0) geo.ImageIndex = 247; // question icon

            return geo;
        }

        /// <summary>
        /// Tries to retrieve the geolocation information locally.
        /// </summary>
        /// <returns>The retrieved geolocation information if successful, otherwise <c>null</c>.</returns>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1614: System Location Discovery
        /// </remarks>
        private GeoInformation TryRetrieveLocally()
        {
            try
            {
                GeoInformation g = new GeoInformation();

                // use local information
                var cultureInfo = CultureInfo.CurrentUICulture;
                var region = new RegionInfo(cultureInfo.LCID);

                g.Country = region.DisplayName;
                g.CountryCode = region.TwoLetterISORegionName;
                g.Timezone = DateTimeHelper.GetLocalTimeZone();

                return g;
            }
            catch
            {
                return null;
            }
        }
    }
}
