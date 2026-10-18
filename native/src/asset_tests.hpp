#pragma once
#include "assets.hpp"
#include <functional>
inline void put32(std::vector<uint8_t>& b, size_t p, uint32_t v) {
    for (int i = 0; i < 4; ++i) b.at(p + i) = uint8_t(v >> (24 - 8*i));
}
inline void assetTests() {
    auto rejects = [](const std::function<void()>& f) {
        bool caught = false; try { f(); } catch (const std::runtime_error&) { caught = true; }
        assetCheck(caught, "Malformed asset unexpectedly accepted");
    };
    // NIST SP 800-38A AES-128 CBC first-block known-answer test.
    uint8_t key[16] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    uint8_t iv[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8_t cipher[16] = {0x76,0x49,0xab,0xac,0x81,0x19,0xb2,0x46,0xce,0xe9,0x8e,0x9b,0x12,0xe9,0x19,0x7d};
    std::vector<uint8_t> plain{0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a};
    AesCbc aes(key);
    assetCheck(aes.decrypt(cipher, 16, iv) == plain, "AES known-answer test failed");
    rejects([&]{ aes.decrypt(cipher, 15, iv); });
    const uint8_t abc[] = {'a','b','c'};
    std::array<uint8_t,20> abcHash{0xa9,0x99,0x3e,0x36,0x47,0x06,0x81,0x6a,0xba,0x3e,0x25,0x71,0x78,0x50,0xc2,0x6c,0x9c,0xd0,0xd8,0x9d};
    assetCheck(sha1(abc, 3) == abcHash, "SHA1 known-answer test failed");
    std::vector<uint8_t> cluster(0x7c00,0x5a), hashes(0x400,0);
    auto chunkHash=sha1(cluster.data(),0x400);
    for (size_t i=0;i<31;++i) std::copy(chunkHash.begin(),chunkHash.end(),hashes.begin()+i*20);
    verifyH0(hashes,cluster);
    cluster.back()^=1; rejects([&]{verifyH0(hashes,cluster);}); cluster.back()^=1;
    hashes[0]^=1; rejects([&]{verifyH0(hashes,cluster);});
    std::vector<uint8_t> fst(40, 0);
    put32(fst,0,0x01000000); put32(fst,8,3);
    put32(fst,12,0x01000000); put32(fst,20,3);
    put32(fst,24,2); put32(fst,28,0x40); put32(fst,32,3);
    fst[36]='d'; fst[38]='f';
    auto files = parseFst(fst,0x200);
    assetCheck(files.size()==1 && files[0].path=="/d/f" && files[0].offset==0x100 && files[0].size==3, "FST nested lookup failed");
    rejects([&]{ parseFst(fst, 0x101); });
    auto broken = fst; put32(broken,20,4); rejects([&]{parseFst(broken,0x200);});
    broken=fst; broken[38]='/'; rejects([&]{parseFst(broken,0x200);});
    broken=fst; broken[39]='x'; rejects([&]{parseFst(broken,0x200);});
    std::vector<uint8_t> yaz(16,0); std::memcpy(yaz.data(),"Yaz0",4); put32(yaz,4,9);
    yaz.insert(yaz.end(), {0xe0,'A','B','C',0x40,2});
    assetCheck(decodeYaz0(yaz)==std::vector<uint8_t>({'A','B','C','A','B','C','A','B','C'}), "Yaz0 overlap copy failed");
    auto badYaz=yaz; badYaz.back()=9; rejects([&]{decodeYaz0(badYaz);});
    badYaz=yaz; badYaz.pop_back(); rejects([&]{decodeYaz0(badYaz);});
    badYaz=yaz; put32(badYaz,4,8); rejects([&]{decodeYaz0(badYaz);});
    std::vector<uint8_t> arc(0xa0,0); std::memcpy(arc.data(),"RARC",4);
    put32(arc,4,0xa0); put32(arc,8,0x20); put32(arc,12,0x60); put32(arc,16,0x20);
    put32(arc,0x20,1); put32(arc,0x24,0x20); put32(arc,0x28,1); put32(arc,0x2c,0x30);
    put32(arc,0x30,2); put32(arc,0x34,0x44); arc[0x4b]=1;
    put32(arc,0x54,0x01000000); put32(arc,0x5c,3); arc[0x64]='f';
    std::copy_n(abc,3,arc.data()+0x80);
    Rarc archive(arc);
    assetCheck(archive.files.size()==1 && archive.readFile("/f")==std::vector<uint8_t>({'a','b','c'}), "RARC resource lookup failed");
    rejects([&]{archive.readFile("/absent");});
    auto badArc=arc; put32(badArc,0x58,0x20); rejects([&]{Rarc a(badArc);});
    badArc=arc; put32(badArc,0x54,0x02000000); rejects([&]{Rarc a(badArc);});
    badArc=arc; put32(badArc,0x34,0xfffffff0); rejects([&]{Rarc a(badArc);});
    std::vector<uint8_t> u8archive(0x63,0);
    put32(u8archive,0,0x55aa382d); put32(u8archive,4,0x20); put32(u8archive,8,26); put32(u8archive,12,0x60);
    put32(u8archive,0x20,0x01000000); put32(u8archive,0x28,2);
    put32(u8archive,0x30,0x60); put32(u8archive,0x34,3); u8archive[0x38]='f';
    std::copy_n(abc,3,u8archive.data()+0x60);
    Rarc u8parsed(u8archive);
    assetCheck(u8parsed.readFile("/f")==std::vector<uint8_t>({'a','b','c'}), "U8 lookup failed");
    put32(u8archive,0x30,0x20); rejects([&]{Rarc a(u8archive);});
}
