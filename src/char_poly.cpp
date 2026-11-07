vector<Mint> characteristic_polynomial(vector<vector<Mint>>& mat){
    int n = int(mat.size());
    if (n == 0) return {Mint(1)};
    vector<Mint> T(n);
    for (int y = 1; y < n; y++) {
        int y1 = y;
        while (y1 < n && mat[y1][y - 1] == 0) y1++;
        if (y1 == n) continue;
        if (y != y1) {
            for (int col = 0; col < n; col++) swap(mat[y][col], mat[y1][col]);
            for (int row = 0; row < n; row++) swap(mat[row][y], mat[row][y1]);
        }
        T[y] = Mint(1) / mat[y][y - 1];
        for (int y2 = y + 1; y2 < n; y2++) T[y2] = T[y] * mat[y2][y - 1];
        for (int y2 = y + 1; y2 < n; y2++) for (int x = y - 1; x < n; x++) mat[y2][x] -= mat[y][x] * T[y2];
        for (int y2 = 0; y2 < n; y2++) for (int x = y + 1; x < n; x++) mat[y2][y] += mat[y2][x] * T[x];
    }
    for (int y = 0; y < n; y++) {
        Mint tmp = 1;
        for (int x = y + 1; x < n; x++) {
            mat[y][x] *= (tmp *= -mat[x][x - 1]);
        }
    }
    vector<vector<Mint>> dp(n+1, vector<Mint>(n + 1, 0));
    dp[0][0] = 1;
    for (int y = 0; y < n; y++) {
        for (int x = 0; x <= y; x++) dp[y + 1][x + 1] -= dp[y][x];
        for (int x = 0; x <= y; x++) dp[y + 1][x] += dp[y][x] * mat[y][y];
        for (int y2 = 0; y2 < y; y2++) for (int x = 0; x <= y2; x++) dp[y + 1][x] +=
dp[y2][x] * mat[y2][y];,
    }
    vector<Mint> res(n + 1);
    for (int i = 0; i <= n; i++) res[i] = ((n % 2 == 1) ? -dp[n][i] : dp[n][i]);
    return res;
}