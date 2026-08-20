#pragma once

#include <Arduino.h>   // needed for PlatformIO
#include <Mesh.h>
#include <helpers/IdentityStore.h>

#ifndef MAX_BLACKLIST
  #define MAX_BLACKLIST         20
#endif

// Persisted list of repeater pubkeys whose presence anywhere in a packet's
// path means the packet should not be repeated.
class Blacklist {
  FILESYSTEM* _fs;
  uint8_t pub_keys[MAX_BLACKLIST][PUB_KEY_SIZE];
  int num_entries;

  int indexOf(const uint8_t* pubkey, int key_len) const;

public:
  Blacklist() {
    memset(pub_keys, 0, sizeof(pub_keys));
    num_entries = 0;
  }

  void load(FILESYSTEM* fs);
  void save(FILESYSTEM* fs);

  bool add(const uint8_t* pubkey, int key_len);
  bool remove(const uint8_t* pubkey, int key_len);

  // true if any blacklisted pubkey's first 'hash_len' bytes match 'hash'
  bool containsHash(const uint8_t* hash, uint8_t hash_len) const;

  int getNumEntries() const { return num_entries; }
  const uint8_t* getEntry(int idx) const { return pub_keys[idx]; }
};
