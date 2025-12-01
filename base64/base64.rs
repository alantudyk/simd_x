const M: &[u8] =
    b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

fn main() {
    fn scalar_encode(s: &[u8], t: &mut [u8]) {
        fn f(b: u8) -> u8 { M[b as usize] }
        let n = s.len();
        let r = n % 3;
        let n = n - r;
        let mut si = 0;
        let mut ti = 0;
        while si < n {
            let a = s[si + 0];
            let b = s[si + 1];
            let c = s[si + 2];
            t[ti + 0] = f(a >> 2);
            t[ti + 1] = f(((a &  3) << 4) | (b >> 4));
            t[ti + 2] = f(((b & 15) << 2) | (c >> 6));
            t[ti + 3] = f(c & 63);
            si += 3;
            ti += 4;
        }
        match r {
            1 => {
                let a = s[si + 0];
                t[ti + 0] = f(a >> 2);
                t[ti + 1] = f((a & 3) << 4);
                t[ti + 2] = b'=';
                t[ti + 3] = b'=';
            },
            2 => {
                let a = s[si + 0];
                let b = s[si + 1];
                t[ti + 0] = f(a >> 2);
                t[ti + 1] = f(((a &  3) << 4) | (b >> 4));
                t[ti + 2] = f((b & 15) << 2);
                t[ti + 3] = b'=';
            },
            _ => (),
        }
    }

    #[allow(unused_variables)]
    fn simd_encode(s: &[u8], t: &mut [u8]) {
        // mask_loadu
        // vpermb
        // magic
        // vpermb
        // storeu
    }

    type U = usize;
    const N: U = 1 << 20;
    const T: U = 1 << 13;
    let mut t = [0; (N + 2) / 3 * 4];

    assert_eq!({ scalar_encode(b"M"  , &mut t); &t[..4] }, b"TQ==");
    assert_eq!({ scalar_encode(b"Ma" , &mut t); &t[..4] }, b"TWE=");
    assert_eq!({ scalar_encode(b"Man", &mut t); &t[..4] }, b"TWFu");

    let s = {
        let mut s = [0_u8; N];
        use std::io::Read;
        std::fs::File::open("/dev/urandom")
            .unwrap()
            .read_exact(&mut s)
            .unwrap()
        ;
        s
    };

    println!("");
    println!("    {} bytes, {} times, average:", N, T);
    println!("");

    let d: Vec<_> = [scalar_encode, simd_encode].iter().map( | f | {
        let d = std::time::SystemTime::now();
        for _ in 0..T { f(&s, &mut t) }
        let d = d.elapsed().unwrap().as_secs_f64();
        assert!(t[N] < 128);
        let d = (N * T) as f64 / 1e6 / d;
        if d < 1e6 { d as usize } else { 0 }
    }).collect();

    println!("        Scalar Encode: {:7} MB/s", d[0]);
    println!("        SIMD   Encode: {:7} MB/s", d[1]);
    
    println!("");
}
