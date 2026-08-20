#include "Blacklist.h"

static File openWrite(FILESYSTEM* _fs, const char* filename) {
  #if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
    _fs->remove(filename);
    return _fs->open(filename, FILE_O_WRITE);
  #elif defined(RP2040_PLATFORM)
    return _fs->open(filename, "w");
  #else
    return _fs->open(filename, "w", true);
  #endif
}

void Blacklist::load(FILESYSTEM* fs) {
  _fs = fs;
  num_entries = 0;
  if (_fs->exists("/blacklist")) {
  #if defined(RP2040_PLATFORM)
    File file = _fs->open("/blacklist", "r");
  #else
    File file = _fs->open("/blacklist");
  #endif
    if (file) {
      while (num_entries < MAX_BLACKLIST) {
        if (file.read(pub_keys[num_entries], PUB_KEY_SIZE) != PUB_KEY_SIZE) break;  // EOF
        num_entries++;
      }
      file.close();
    }
  }
}

void Blacklist::save(FILESYSTEM* fs) {
  _fs = fs;
  File file = openWrite(_fs, "/blacklist");
  if (file) {
    for (int i = 0; i < num_entries; i++) {
      if (file.write(pub_keys[i], PUB_KEY_SIZE) != PUB_KEY_SIZE) break;  // write failed
    }
    file.close();
  }
}

int Blacklist::indexOf(const uint8_t* pubkey, int key_len) const {
  for (int i = 0; i < num_entries; i++) {
    if (memcmp(pubkey, pub_keys[i], key_len) == 0) return i;
  }
  return -1;
}

bool Blacklist::add(const uint8_t* pubkey, int key_len) {
  if (key_len < PUB_KEY_SIZE) return false;   // need complete pubkey to add
  if (indexOf(pubkey, PUB_KEY_SIZE) >= 0) return true;   // already blacklisted
  if (num_entries >= MAX_BLACKLIST) return false;   // full

  memcpy(pub_keys[num_entries++], pubkey, PUB_KEY_SIZE);
  return true;
}

bool Blacklist::remove(const uint8_t* pubkey, int key_len) {
  int idx = indexOf(pubkey, key_len);
  if (idx < 0) return false;   // not found

  num_entries--;
  while (idx < num_entries) {
    memcpy(pub_keys[idx], pub_keys[idx + 1], PUB_KEY_SIZE);
    idx++;
  }
  return true;
}

bool Blacklist::containsHash(const uint8_t* hash, uint8_t hash_len) const {
  for (int i = 0; i < num_entries; i++) {
    if (memcmp(hash, pub_keys[i], hash_len) == 0) return true;
  }
  return false;
}
