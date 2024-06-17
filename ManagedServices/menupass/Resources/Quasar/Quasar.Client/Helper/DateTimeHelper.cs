using System;

namespace Quasar.Client.Helper
{
    public static class DateTimeHelper
    {
        /// <summary>
        /// Gets local time zone info.
        /// </summary>
        /// <remarks>
        /// MITRE ATT&amp;CK Techniques:<br/>
        /// - T1124: System Time Discovery
        /// </remarks>
        public static string GetLocalTimeZone()
        {
            var tz = TimeZoneInfo.Local;
            var tzOffset = tz.GetUtcOffset(DateTime.Now);
            var tzOffsetSign = tzOffset >= TimeSpan.Zero ? "+" : "";
            var tzName = tz.SupportsDaylightSavingTime && tz.IsDaylightSavingTime(DateTime.Now) ? tz.DaylightName : tz.StandardName;
            return $"{tzName} (UTC {tzOffsetSign}{tzOffset.Hours}{(tzOffset.Minutes != 0 ? $":{Math.Abs(tzOffset.Minutes)}" : "")})";
        }
    }
}
