namespace StudioTVPlayer.Helpers
{
    public class Wildcard
    {
        private readonly string _pattern;

        public Wildcard(string pattern)
        {
            _pattern = pattern;
        }

        public static bool Match(string value, string pattern)
        {
            int start = -1;
            int end = -1;
            return Match(value, pattern, ref start, ref end);
        }

        public static bool Match(string value, string pattern, char[] toLowerTable)
        {
            int start = -1;
            int end = -1;
            return Match(value, pattern, ref start, ref end, toLowerTable);
        }

        public static bool Match(string value, string pattern, ref int start, ref int end)
        {
            return new Wildcard(pattern).IsMatch(value, ref start, ref end);
        }

        public static bool Match(string value, string pattern, ref int start, ref int end, char[] toLowerTable)
        {
            return new Wildcard(pattern).IsMatch(value, ref start, ref end, toLowerTable);
        }

        public bool IsMatch(string str)
        {
            int start = -1;
            int end = -1;
            return IsMatch(str, ref start, ref end);
        }

        public bool IsMatch(string str, char[] toLowerTable)
        {
            int start = -1;
            int end = -1;
            return IsMatch(str, ref start, ref end, toLowerTable);
        }

        public bool IsMatch(string str, ref int start, ref int end)
        {
            if (_pattern.Length == 0) return false;
            int pindex = 0;
            int sindex = 0;
            int pattern_len = _pattern.Length;
            int str_len = str.Length;
            start = -1;
            while (true)
            {
                bool star = false;
                if (_pattern[pindex] == '*')
                {
                    star = true;
                    do
                    {
                        pindex++;
                    }
                    while (pindex < pattern_len && _pattern[pindex] == '*');
                }
                end = sindex;
                int i;
                while (true)
                {
                    int si = 0;
                    bool breakLoops = false;
                    for (i = 0; pindex + i < pattern_len && _pattern[pindex + i] != '*'; i++)
                    {
                        si = sindex + i;
                        if (si == str_len)
                        {
                            return false;
                        }
                        if (str[si] == _pattern[pindex + i])
                        {
                            continue;
                        }
                        if (si == str_len)
                        {
                            return false;
                        }
                        if (_pattern[pindex + i] == '?' && str[si] != '.')
                        {
                            continue;
                        }
                        breakLoops = true;
                        break;
                    }
                    if (breakLoops)
                    {
                        if (!star)
                        {
                            return false;
                        }
                        sindex++;
                        if (si == str_len)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (start == -1)
                        {
                            start = sindex;
                        }
                        if (pindex + i < pattern_len && _pattern[pindex + i] == '*')
                        {
                            break;
                        }
                        if (sindex + i == str_len)
                        {
                            if (end <= start)
                            {
                                end = str_len;
                            }
                            return true;
                        }
                        if (i != 0 && _pattern[pindex + i - 1] == '*')
                        {
                            return true;
                        }
                        if (!star)
                        {
                            return false;
                        }
                        sindex++;
                    }
                }
                sindex += i;
                pindex += i;
                if (start == -1)
                {
                    start = sindex;
                }
            }
        }

        public bool IsMatch(string str, ref int start, ref int end, char[] toLowerTable)
        {
            if (_pattern.Length == 0) return false;

            int pindex = 0;
            int sindex = 0;
            int pattern_len = _pattern.Length;
            int str_len = str.Length;
            start = -1;
            while (true)
            {
                bool star = false;
                if (_pattern[pindex] == '*')
                {
                    star = true;
                    do
                    {
                        pindex++;
                    }
                    while (pindex < pattern_len && _pattern[pindex] == '*');
                }
                end = sindex;
                int i;
                while (true)
                {
                    int si = 0;
                    bool breakLoops = false;

                    for (i = 0; pindex + i < pattern_len && _pattern[pindex + i] != '*'; i++)
                    {
                        si = sindex + i;
                        if (si == str_len)
                        {
                            return false;
                        }
                        char c = toLowerTable[str[si]];
                        if (c == _pattern[pindex + i])
                        {
                            continue;
                        }
                        if (si == str_len)
                        {
                            return false;
                        }
                        if (_pattern[pindex + i] == '?' && c != '.')
                        {
                            continue;
                        }
                        breakLoops = true;
                        break;
                    }
                    if (breakLoops)
                    {
                        if (!star)
                        {
                            return false;
                        }
                        sindex++;
                        if (si == str_len)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (start == -1)
                        {
                            start = sindex;
                        }
                        if (pindex + i < pattern_len && _pattern[pindex + i] == '*')
                        {
                            break;
                        }
                        if (sindex + i == str_len)
                        {
                            if (end <= start)
                            {
                                end = str_len;
                            }
                            return true;
                        }
                        if (i != 0 && _pattern[pindex + i - 1] == '*')
                        {
                            return true;
                        }
                        if (!star)
                        {
                            return false;
                        }
                        sindex++;
                        continue;
                    }
                }
                sindex += i;
                pindex += i;
                if (start == -1)
                {
                    start = sindex;
                }
            }
        }
    }
}
