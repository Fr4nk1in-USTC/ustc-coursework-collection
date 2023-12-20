import argparse
import random
import time
from itertools import repeat
from multiprocessing.pool import ThreadPool
from typing import Optional

try:
    import tqdm
except ImportError:
    tqdm = None
import sympy


def is_primitive_root(g, p, factors):
    # determine whether g is a primitive root of p
    for factor in factors:
        if pow(g, (p - 1) // factor, p) == 1:
            return False
    return True


def generate_p_and_g(n_bit):
    while True:
        # generate an n-bit random prime number p
        p = sympy.randprime(2 ** (n_bit - 1), 2**n_bit)

        # compute the prime factorization of p-1
        factors = sympy.factorint(p - 1).keys()

        # choose a possible primitive root g
        for g in range(2, p):
            if is_primitive_root(g, p, factors):
                return p, g


def mod_exp(base, exponent, modulus):
    """TODO: calculate (base^exponent) mod modulus.
    Recommend to use the fast power algorithm.
    """
    # It's slow to write the code by hand, simple `pow` is enough.
    # if exponent == 0:
    #     return 1
    # if exponent == 1:
    #     return base % modulus
    # if exponent % 2 == 0:
    #     return (mod_exp(base, exponent // 2, modulus) ** 2) % modulus
    # if exponent % 2 == 1:
    #     return (mod_exp(base, exponent // 2, modulus) ** 2 * base) % modulus
    return pow(base, exponent, modulus)


def elgamal_key_generation(key_size):
    """Generate the keys based on the key_size."""
    # generate a large prime number p and a primitive root g
    p, g = generate_p_and_g(key_size)

    # TODO: generate x and y here.
    x = random.randint(1, p - 2)
    y = mod_exp(g, x, p)

    return (p, g, y), x


def elgamal_encrypt(public_key, plaintext):
    """TODO: encrypt the plaintext with the public key."""
    # unpack public key
    p, g, y = public_key
    # check if plaintext is smaller than p
    if plaintext >= p:
        raise ValueError("plaintext should be smaller than p")
    # choose a temporary secret key k
    k = random.randint(1, p - 2)
    c1 = mod_exp(g, k, p)  # temporary public key
    c2 = plaintext * mod_exp(y, k, p) % p  # temporary ciphertext
    return c1, c2


def elgamal_decrypt(public_key, private_key, ciphertext):
    """TODO: decrypt the ciphertext with the public key and the private key."""
    # unpack public key and ciphertext
    p, _, _ = public_key
    c1, c2 = ciphertext

    s = mod_exp(c1, private_key, p)
    s_inv = sympy.mod_inverse(s, p)  # modular inverse of s
    plaintext = c2 * s_inv % p
    return plaintext


def elgamal_encrypt_batch(public_key, plaintexts):
    """Encrypt a batch of plaintexts."""
    # multiprocessing
    with ThreadPool() as pool:
        return pool.starmap(elgamal_encrypt, zip(repeat(public_key), plaintexts))


def elgamal_decrypt_batch(public_key, private_key, ciphertexts):
    """Decrypt a batch of ciphertexts."""
    # multiprocessing
    with ThreadPool() as pool:
        return pool.starmap(
            elgamal_decrypt, zip(repeat(public_key), repeat(private_key), ciphertexts)
        )


def interactive_demo() -> None:
    # set key_size, such as 256, 1024...
    key_size = int(input("Please input the key size: "))

    # generate keys
    public_key, private_key = elgamal_key_generation(key_size)
    p, _, _ = public_key
    print("Public Key:", public_key)
    print("Private Key:", private_key)

    # encrypt plaintext
    plaintext = 0
    while not 0 < plaintext < p - 1:
        plaintext = int(input(f"Please input an integer m (0 < m < {p - 1}): "))
    ciphertext = elgamal_encrypt(public_key, plaintext)
    print("Ciphertext:", ciphertext)

    # decrypt ciphertext
    decrypted_text = elgamal_decrypt(public_key, private_key, ciphertext)
    print("Decrypted Text:", decrypted_text)


def profile_simple(key_size: int, repeat: int = 100) -> tuple[float, float, float]:
    """
    Profile the key generation, encryption and decryption time for a key size.
    Repeat the profiling for `repeat` times and return the average time of the
    three phases.
    """
    total_key_gen_time = 0.0
    total_enc_time = 0.0
    total_dec_time = 0.0

    if tqdm is None:
        iters = range(repeat)
    else:
        iters = tqdm.trange(repeat)
    for _ in iters:
        # Key generation
        key_gen_start = time.perf_counter()
        public_key, private_key = elgamal_key_generation(key_size)
        key_gen_end = time.perf_counter()
        # Encryption
        p, _, _ = public_key
        plaintext = random.randint(1, p - 2)
        enc_start = time.perf_counter()
        ciphertext = elgamal_encrypt(public_key, plaintext)
        enc_end = time.perf_counter()
        # Decryption
        dec_start = time.perf_counter()
        decrypted_text = elgamal_decrypt(public_key, private_key, ciphertext)
        dec_end = time.perf_counter()
        # Check correctness
        assert decrypted_text == plaintext

        total_key_gen_time += key_gen_end - key_gen_start
        total_enc_time += enc_end - enc_start
        total_dec_time += dec_end - dec_start

    return (
        total_key_gen_time / repeat,
        total_enc_time / repeat,
        total_dec_time / repeat,
    )


def profile_batch(
    key_size: int, repeat: int = 5, batch_size: int = 2**30
) -> tuple[float, float, float]:
    """
    Profile the key generation, batch encryption and batch decryption time for
    a key size. Repeat the profiling for `repeat` times and return the average
    time of the three phases.
    """
    total_key_gen_time = 0.0
    total_enc_time = 0.0
    total_dec_time = 0.0
    if tqdm is None:
        iters = range(repeat)
    else:
        iters = tqdm.trange(repeat)
    for _ in iters:
        # Key generation
        key_gen_start = time.perf_counter()
        public_key, private_key = elgamal_key_generation(key_size)
        key_gen_end = time.perf_counter()
        # Encryption
        p, _, _ = public_key
        with ThreadPool() as pool:
            plaintexts = pool.starmap(random.randint, [(1, p - 2)] * batch_size)
        enc_start = time.perf_counter()
        ciphertexts = elgamal_encrypt_batch(public_key, plaintexts)
        enc_end = time.perf_counter()
        # Decryption
        dec_start = time.perf_counter()
        decrypted_texts = elgamal_decrypt_batch(public_key, private_key, ciphertexts)
        dec_end = time.perf_counter()
        # Check correctness
        assert decrypted_texts == plaintexts

        total_key_gen_time += key_gen_end - key_gen_start
        total_enc_time += enc_end - enc_start
        total_dec_time += dec_end - dec_start

    return (
        total_key_gen_time / repeat,
        total_enc_time / repeat,
        total_dec_time / repeat,
    )


def profile_multiplicative_homomorphism(
    key_size: int, repeat: int = 5
) -> tuple[float, float]:
    """
    Profile time of `decrypt(a) * decrypt(b)` and `decrypt(a * b)` for a key
    size. `a` and `b` are two ciphertexts encrypted from two plaintexts. Repeat
    the profiling for `repeat` times and return the average time of the two
    phases.
    """
    total_dec_and_mul_time = 0.0
    total_dec_mul_time = 0.0
    if tqdm is None:
        iters = range(repeat)
    else:
        iters = tqdm.trange(repeat)
    for _ in iters:
        public_key, private_key = elgamal_key_generation(key_size)
        p, _, _ = public_key
        # Encryption
        plaintext_1 = random.randint(1, p - 2)
        plaintext_2 = random.randint(1, p - 2)
        ciphertext_1 = elgamal_encrypt(public_key, plaintext_1)
        ciphertext_2 = elgamal_encrypt(public_key, plaintext_2)
        # Decryption and multiplication
        dec_and_mul_start = time.perf_counter()
        decrypted_text_1 = elgamal_decrypt(public_key, private_key, ciphertext_1)
        decrypted_text_2 = elgamal_decrypt(public_key, private_key, ciphertext_2)
        decrypted_text_mul = decrypted_text_1 * decrypted_text_2 % p
        dec_and_mul_end = time.perf_counter()
        # Decryption of multiplication
        dec_mul_start = time.perf_counter()
        ciphertext_mul = (
            ciphertext_1[0] * ciphertext_2[0] % p,
            ciphertext_1[1] * ciphertext_2[1] % p,
        )
        decrypted_text_mul = elgamal_decrypt(public_key, private_key, ciphertext_mul)
        dec_mul_end = time.perf_counter()
        # Check correctness
        assert decrypted_text_mul == decrypted_text_1 * decrypted_text_2 % p

        total_dec_and_mul_time += dec_and_mul_end - dec_and_mul_start
        total_dec_mul_time += dec_mul_end - dec_mul_start

    return (
        total_dec_and_mul_time / repeat,
        total_dec_mul_time / repeat,
    )


def profile(
    key_sizes: list[int],
    repeat: int,
    mode: Optional[str] = None,
    batch_size: Optional[int] = None,
):
    if mode == "simple":
        for key_size in key_sizes:
            key_gen_time, enc_time, dec_time = profile_simple(key_size, repeat)
            print(
                f"Key size:            {key_size}\n"
                f"Key generation time: {key_gen_time}s\n"
                f"Encryption time:     {enc_time}s\n"
                f"Decryption time:     {dec_time}s"
            )
    elif mode == "batch":
        if batch_size is None:
            raise ValueError("batch_size should not be None")
        for key_size in key_sizes:
            key_gen_time, enc_time, dec_time = profile_batch(
                key_size, repeat, batch_size
            )
            print(
                f"Key size:              {key_size}\n"
                f"Batch size:            {args.batch_size}\n"
                f"Key generation time:   {key_gen_time}s\n"
                f"Batch encryption time: {enc_time}s\n"
                f"Batch decryption time: {dec_time}s"
            )
    elif mode == "homo":
        for key_size in key_sizes:
            dec_and_mul_time, mul_and_dec_time = profile_multiplicative_homomorphism(
                key_size, repeat
            )
            print(
                f"Key size:              {key_size}\n"
                f"time(dec(a) * dec(b)): {dec_and_mul_time}s\n"
                f"time(dec(a * b)):      {mul_and_dec_time}s\n"
            )
    else:
        raise ValueError(f"Unknown profile mode: {mode}")


def verify_randomness():
    """Verify the randomness property of ElGamal Encryption."""
    key_size = int(input("Please input the key size: "))
    # generate keys
    public_key, private_key = elgamal_key_generation(key_size)
    p, _, _ = public_key
    print("Public Key:", public_key)
    print("Private Key:", private_key)
    # encrypt plaintext using different k
    plaintext = 0
    while not 0 < plaintext < p - 1:
        plaintext = int(input(f"Please input an integer m (0 < m < {p - 1}): "))
    ciphertext_1 = elgamal_encrypt(public_key, plaintext)
    ciphertext_2 = elgamal_encrypt(public_key, plaintext)
    # check if c1 != c2
    while ciphertext_1 == ciphertext_2:
        ciphertext_2 = elgamal_encrypt(public_key, plaintext)
    print("Ciphertext 1:", ciphertext_1)
    print("Ciphertext 2:", ciphertext_2)
    # decrypt ciphertexts
    decrypted_text_1 = elgamal_decrypt(public_key, private_key, ciphertext_1)
    decrypted_text_2 = elgamal_decrypt(public_key, private_key, ciphertext_2)
    print("Decrypted Text 1:", decrypted_text_1)
    print("Decrypted Text 2:", decrypted_text_2)


def verify_multiplicative_homomorphism():
    """Verify the multiplicative homomorphism property of ElGamal Encryption."""
    key_size = int(input("Please input the key size: "))
    # generate keys
    public_key, private_key = elgamal_key_generation(key_size)
    p, _, _ = public_key
    print("Public Key:", public_key)
    print("Private Key:", private_key)
    # encrypt plaintexts
    plaintext_1 = 0
    while not 0 < plaintext_1 < p - 1:
        plaintext_1 = int(input(f"Please input an integer m1 (0 < m1 < {p - 1}): "))
    plaintext_2 = 0
    while not 0 < plaintext_2 < p - 1:
        plaintext_2 = int(input(f"Please input an integer m2 (0 < m2 < {p - 1}): "))
    print("m1 * m2 % p:", plaintext_1 * plaintext_2 % p)
    ciphertext_1 = elgamal_encrypt(public_key, plaintext_1)
    ciphertext_2 = elgamal_encrypt(public_key, plaintext_2)
    ciphertext_mul = (
        ciphertext_1[0] * ciphertext_2[0] % p,
        ciphertext_1[1] * ciphertext_2[1] % p,
    )
    print("Ciphertext 1:", ciphertext_1)
    print("Ciphertext 2:", ciphertext_2)
    print(
        "Ciphertext 1 * Ciphertext 2:",
        ciphertext_mul,
    )
    # decrypt ciphertexts
    decrypted_text_1 = elgamal_decrypt(public_key, private_key, ciphertext_1)
    decrypted_text_2 = elgamal_decrypt(public_key, private_key, ciphertext_2)
    decrypted_text_mul = elgamal_decrypt(public_key, private_key, ciphertext_mul)
    print("Decrypted Text 1:", decrypted_text_1)
    print("Decrypted Text 2:", decrypted_text_2)
    print("Decrypted Text of Multiplied Ciphertext:", decrypted_text_mul)


def add_args_to_profile_parser(parser: argparse.ArgumentParser):
    parser.add_argument(
        "-k",
        "--key-sizes",
        nargs="+",
        type=int,
        default=[8, 16, 32, 64, 128],
        help="key sizes to profile",
    )
    parser.add_argument(
        "-r",
        "--repeat",
        type=int,
        default=100,
        help="times to repeat the profiling for each key size",
    )


def parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(
        dest="command", metavar="command", help="sub-command"
    )
    # interactive demo
    subparsers.add_parser("interact", help="interactive demo")

    # profile
    profile_parser = subparsers.add_parser("profile", help="profile ElGamal Encryption")
    profile_subparsers = profile_parser.add_subparsers(
        dest="profile_mode", metavar="mode", help="profile mode"
    )

    simple_profile_parser = profile_subparsers.add_parser(
        "simple", help="simple profile (no batch)"
    )
    add_args_to_profile_parser(simple_profile_parser)

    batch_profile_parser = profile_subparsers.add_parser(
        "batch", help="profile batch encryption and batch decryption"
    )
    add_args_to_profile_parser(batch_profile_parser)
    batch_profile_parser.add_argument(
        "-b",
        "--batch-size",
        type=int,
        default=1024,
        help="batch size for the batch encryption and batch decryption",
    )

    homo_profile_parser = profile_subparsers.add_parser(
        "homo",
        help="profile multiplicative homomorphism (time of `dec(a * b)` and `dec(a) * dec(b))`",
    )
    add_args_to_profile_parser(homo_profile_parser)

    # verify randomness & multiplicative homomorphism
    verify_parser = subparsers.add_parser(
        "verify", help="verify property of ElGamal Encryption"
    )
    verify_subparsers = verify_parser.add_subparsers(
        dest="property", metavar="property", help="property to verify"
    )
    verify_subparsers.add_parser("random", help="verify randomness")
    verify_subparsers.add_parser("homo", help="verify multiplicative homomorphism")

    return parser


if __name__ == "__main__":
    args = parser().parse_args()

    if args.command == "interact" or args.command is None:
        interactive_demo()
    elif args.command == "profile":
        if args.profile_mode is None:
            raise ValueError("Please provide a profile mode from [simple, batch, homo]")
        try:
            batch_size = args.batch_size
        except Exception as _:
            batch_size = None
        profile(
            mode=args.profile_mode,
            key_sizes=args.key_sizes,
            repeat=args.repeat,
            batch_size=batch_size,
        )
    elif args.command == "verify":
        if args.property == "random":
            verify_randomness()
        elif args.property == "homo":
            verify_multiplicative_homomorphism()
        else:
            raise ValueError(f"Unknown property: {args.property}")
    else:
        raise ValueError(f"Unknown command: {args.command}")
