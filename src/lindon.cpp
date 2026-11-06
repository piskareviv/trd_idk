// Lyndon: // String is simple, if it is smaller than all its suffixes
// s = s_1 s_2 ... s_k, s_1 >= s_2 >= ... >= s_k, s_i is simple
std::string min_cyclic_shift (std::string s) {
    s += s; int n = (int) s.length(), i=0, ans=0;
    while (i < n/2) {
        ans = i; int j=i+1, k=i;
        while (j < n && s[k] <= s[j]) {
            if (s[k] < s[j]) {k = i;} else {++k;}
            ++j;
        } while (i <= k) i += j - k;
    }
    return s.substr(ans, n/2);
}
