// FastDelaunay.h
// Description: Fast Delaunay triangulation. Each circumcircle contains none
// of the input points. There must be no duplicate points. If all points are on a
// line, no triangles will be returned. Should work for doubles as well, though
// there may be precision issues in circ. Returns triangles in order {t[0][0],
// t[0][1], t[0][2], t[1][0], . . . }, all counter-clockwise.
// Time: O (n log n)
// "Point.h" eefdf5, 88 lines

template <class T>
struct Point3D {
    typedef Point3D P;
    typedef const P& R;
    T x, y, z;
    explicit Point3D(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
    bool operator<(R p) const {
        return tie(x, y, z) < tie(p.x, p.y, p.z);
    }
    bool operator==(R p) const {
        return tie(x, y, z) == tie(p.x, p.y, p.z);
    }
    P operator+(R p) const { return P(x + p.x, y + p.y, z + p.z); }
    P operator-(R p) const { return P(x - p.x, y - p.y, z - p.z); }
    P operator*(T d) const { return P(x * d, y * d, z * d); }
    P operator/(T d) const { return P(x / d, y / d, z / d); }
    T dot(R p) const { return x * p.x + y * p.y + z * p.z; }
    P cross(R p) const {
        return P(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
    }
    T dist2() const { return x * x + y * y + z * z; }
    double dist() const { return sqrt((double)dist2()); }
    // Azimuthal angle ( longitude ) to x=axis in interval [=pi , pi ]
    double phi() const { return atan2(y, x); }
    // Zenith angle ( l a t i t u d e ) to the z=axis in interval [0 , pi ]
    double theta() const { return atan2(sqrt(x * x + y * y), z); }
    P unit() const { return *this / (T)dist(); }  // makes d i s t ()=1
    // returns unit vector normal to * t h i s and p
    P normal(P p) const { return cross(p).unit(); }
    // returns point rotated " angle " radians ccw around axis
    P rotate(double angle, P axis) const {
        double s = sin(angle), c = cos(angle);
        P u = axis.unit();
        return u * dot(u) * (1 - c) + (*this) * c - cross(u) * s;
    }
};

typedef Point<ll> P;
typedef struct Quad* Q;
typedef __int128_t lll;       // (can be l l i f coords are < 2e4)
P arb(LLONG_MAX, LLONG_MAX);  // not equal to any other point
struct Quad {
    Q rot, o;
    P p = arb;
    bool mark;
    P& F() { return r()->p; }
    Q& r() { return rot->rot; }
    Q prev() { return rot->o->rot; }
    Q next() { return r()->prev(); }
}* H;
bool circ(P p, P a, P b, P c) {  // i s p in the circumcircle?
    lll p2 = p.dist2(), A = a.dist2() - p2,
        B = b.dist2() - p2, C = c.dist2() - p2;
    return p.cross(a, b) * C + p.cross(b, c) * A + p.cross(c, a) * B > 0;
}
Q makeEdge(P orig, P dest) {
    Q r = H ? H : new Quad{new Quad{new Quad{new Quad{0}}}};
    H = r->o;
    r->r()->r() = r;
    rep(i, 0, 4) r = r->rot, r->p = arb, r->o = i & 1 ? r : r->r();
    r->p = orig;
    r->F() = dest;
    return r;
}
void splice(Q a, Q b) {
    swap(a->o->rot->o, b->o->rot->o);
    swap(a->o, b->o);
}
Q connect(Q a, Q b) {
    Q q = makeEdge(a->F(), b->p);
    splice(q, a->next());
    splice(q->r(), b);
    return q;
}
pair<Q, Q> rec(const vector<P>& s) {
    if (sz(s) <= 3) {
        Q a = makeEdge(s[0], s[1]), b = makeEdge(s[1], s.back());
        if (sz(s) == 2) return {a, a->r()};
        KTH PolyhedronVolume Point3D 3dHull sphericalDistance KMP Zfunc 20 splice(a->r(), b);
        auto side = s[0].cross(s[1], s[2]);
        Q c = side ? connect(b, a) : 0;
        return {side < 0 ? c->r() : a, side < 0 ? c : b->r()};
    }
#define H(e) e->F(), e->p
#define valid(e) (e->F().cross(H(base)) > 0)
    Q A, B, ra, rb;
    int half = sz(s) / 2;
    tie(ra, A) = rec({all(s) - half});
    tie(B, rb) = rec({sz(s) - half + all(s)});
    while ((B->p.cross(H(A)) < 0 && (A = A->next())) ||
           (A->p.cross(H(B)) > 0 && (B = B->r()->o)));
    Q base = connect(B->r(), A);
    if (A->p == ra->p) ra = base->r();
    if (B->p == rb->p) rb = base;
#define DEL(e, init, dir)                            \
    Q e = init->dir;                                 \
    if (valid(e))                                    \
        while (circ(e->dir->F(), H(base), e->F())) { \
            Q t = e->dir;                            \
            splice(e, e->prev());                    \
            splice(e->r(), e->r()->prev());          \
            e->o = H;                                \
            H = e;                                   \
            e = t;                                   \
        }
    for (;;) {
        DEL(LC, base->r(), o);
        DEL(RC, base, prev());
        if (!valid(LC) && !valid(RC)) break;
        if (!valid(LC) || (valid(RC) && circ(H(RC), H(LC))))
            base = connect(RC, base->r());
        else
            base = connect(base->r(), LC->r());
    }
    return {ra, rb};
}
vector<P> triangulate(vector<P> pts) {
    sort(all(pts));
    assert(unique(all(pts)) == pts.end());
    if (sz(pts) < 2) return {};
    Q e = rec(pts).first;
    vector<Q> q = {e};
    int qi = 0;
    while (e->o->F().cross(e->F(), e->p) < 0) e = e->o;
#define ADD                      \
    {                            \
        Q c = e;                 \
        do {                     \
            c->mark = 1;         \
            pts.push_back(c->p); \
            q.push_back(c->r()); \
            c = c->next();       \
        } while (c != e);        \
    }
    ADD;
    pts.clear();
    while (qi < sz(q))
        if (!(e = q[qi++])->mark) ADD;
    return pts;
}

// 3dHull.h
// Description: Computes all faces of the 3-dimension hull of a point set. *No
// four points must be coplanar*, or else random results will be returned. All
// faces will point outwards.
// Time: O n2
// "Point3D.h" 5b45fc, 49 lines
typedef Point3D<double> P3;
struct PR {
    void ins(int x) { (a == -1 ? a : b) = x; }
    void rem(int x) { (a == x ? a : b) = -1; }
    int cnt() { return (a != -1) + (b != -1); }
    int a, b;
};
struct F {
    P3 q;
    int a, b, c;
};
vector<F> hull3d(const vector<P3>& A) {
    assert(sz(A) >= 4);
    vector<vector<PR>> E(sz(A), vector<PR>(sz(A), {-1, -1}));
#define E(x, y) E[f.x][f.y]
    vector<F> FS;
    auto mf = [&](int i, int j, int k, int l) {
        P3 q = (A[j] - A[i]).cross((A[k] - A[i]));
        if (q.dot(A[l]) > q.dot(A[i]))
            q = q * -1;
        F f{q, i, j, k};
        E(a, b).ins(k);
        E(a, c).ins(j);
        E(b, c).ins(i);
        FS.push_back(f);
    };
    rep(i, 0, 4) rep(j, i + 1, 4) rep(k, j + 1, 4)
        mf(i, j, k, 6 - i - j - k);
    rep(i, 4, sz(A)) {
        rep(j, 0, sz(FS)) {
            F f = FS[j];
            if (f.q.dot(A[i]) > f.q.dot(A[f.a])) {
                E(a, b).rem(f.c);
                E(a, c).rem(f.b);
                E(b, c).rem(f.a);
                swap(FS[j--], FS.back());
                FS.pop_back();
            }
        }
        int nw = sz(FS);
        rep(j, 0, nw) {
            F f = FS[j];
#define C(a, b, c) \
    if (E(a, b).cnt() != 2) mf(f.a, f.b, i, f.c);
            C(a, b, c);
            C(a, c, b);
            C(b, c, a);
        }
    }
    for (F& it : FS)
        if ((A[it.b] - A[it.a]).cross(A[it.c] - A[it.a]).dot(it.q) <= 0) swap(it.c, it.b);
    return FS;
};
// sphericalDistance.h
// Description: Returns the shortest distance on the sphere with radius ra-
// dius between the points with azimuthal angles (longitude) f1 (\phi 1) and f2 (\phi 2)
// from x axis and zenith angles (latitude) t1 (\theta 1) and t2 (\theta 2) from z axis (0 =
// north pole). All angles measured in radians. The algorithm starts by con-
// verting the spherical coordinates to cartesian coordinates so if that is what
// you have you can use only the two last rows. dx*radius is then the difference
// between the two points in the x direction and d*radius is the total distance
// between the points. 611f07, 8 lines
double sphericalDistance(double f1, double t1,
                         double f2, double t2, double radius) {
    double dx = sin(t2) * cos(f2) - sin(t1) * cos(f1);
    double dy = sin(t2) * sin(f2) - sin(t1) * sin(f1);
    double dz = cos(t2) - cos(t1);
    double d = sqrt(dx * dx + dy * dy + dz * dz);
    return radius * 2 * asin(d / 2);
}