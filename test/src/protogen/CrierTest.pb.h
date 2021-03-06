// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: CrierTest.proto

#ifndef PROTOBUF_CrierTest_2eproto__INCLUDED
#define PROTOBUF_CrierTest_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace crier {
namespace test {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_CrierTest_2eproto();
void protobuf_AssignDesc_CrierTest_2eproto();
void protobuf_ShutdownFile_CrierTest_2eproto();

class test_msg_1;
class test_msg_2;
class root_msg;

// ===================================================================

class test_msg_1 : public ::google::protobuf::Message {
 public:
  test_msg_1();
  virtual ~test_msg_1();

  test_msg_1(const test_msg_1& from);

  inline test_msg_1& operator=(const test_msg_1& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const test_msg_1& default_instance();

  void Swap(test_msg_1* other);

  // implements Message ----------------------------------------------

  test_msg_1* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const test_msg_1& from);
  void MergeFrom(const test_msg_1& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required uint32 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::uint32 id() const;
  inline void set_id(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:crier.test.test_msg_1)
 private:
  inline void set_has_id();
  inline void clear_has_id();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 id_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_CrierTest_2eproto();
  friend void protobuf_AssignDesc_CrierTest_2eproto();
  friend void protobuf_ShutdownFile_CrierTest_2eproto();

  void InitAsDefaultInstance();
  static test_msg_1* default_instance_;
};
// -------------------------------------------------------------------

class test_msg_2 : public ::google::protobuf::Message {
 public:
  test_msg_2();
  virtual ~test_msg_2();

  test_msg_2(const test_msg_2& from);

  inline test_msg_2& operator=(const test_msg_2& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const test_msg_2& default_instance();

  void Swap(test_msg_2* other);

  // implements Message ----------------------------------------------

  test_msg_2* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const test_msg_2& from);
  void MergeFrom(const test_msg_2& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string data = 1;
  inline bool has_data() const;
  inline void clear_data();
  static const int kDataFieldNumber = 1;
  inline const ::std::string& data() const;
  inline void set_data(const ::std::string& value);
  inline void set_data(const char* value);
  inline void set_data(const char* value, size_t size);
  inline ::std::string* mutable_data();
  inline ::std::string* release_data();
  inline void set_allocated_data(::std::string* data);

  // @@protoc_insertion_point(class_scope:crier.test.test_msg_2)
 private:
  inline void set_has_data();
  inline void clear_has_data();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::std::string* data_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_CrierTest_2eproto();
  friend void protobuf_AssignDesc_CrierTest_2eproto();
  friend void protobuf_ShutdownFile_CrierTest_2eproto();

  void InitAsDefaultInstance();
  static test_msg_2* default_instance_;
};
// -------------------------------------------------------------------

class root_msg : public ::google::protobuf::Message {
 public:
  root_msg();
  virtual ~root_msg();

  root_msg(const root_msg& from);

  inline root_msg& operator=(const root_msg& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const root_msg& default_instance();

  void Swap(root_msg* other);

  // implements Message ----------------------------------------------

  root_msg* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const root_msg& from);
  void MergeFrom(const root_msg& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .crier.test.test_msg_1 test_msg_1_field = 1;
  inline bool has_test_msg_1_field() const;
  inline void clear_test_msg_1_field();
  static const int kTestMsg1FieldFieldNumber = 1;
  inline const ::crier::test::test_msg_1& test_msg_1_field() const;
  inline ::crier::test::test_msg_1* mutable_test_msg_1_field();
  inline ::crier::test::test_msg_1* release_test_msg_1_field();
  inline void set_allocated_test_msg_1_field(::crier::test::test_msg_1* test_msg_1_field);

  // optional .crier.test.test_msg_2 test_msg_2_field = 2;
  inline bool has_test_msg_2_field() const;
  inline void clear_test_msg_2_field();
  static const int kTestMsg2FieldFieldNumber = 2;
  inline const ::crier::test::test_msg_2& test_msg_2_field() const;
  inline ::crier::test::test_msg_2* mutable_test_msg_2_field();
  inline ::crier::test::test_msg_2* release_test_msg_2_field();
  inline void set_allocated_test_msg_2_field(::crier::test::test_msg_2* test_msg_2_field);

  // @@protoc_insertion_point(class_scope:crier.test.root_msg)
 private:
  inline void set_has_test_msg_1_field();
  inline void clear_has_test_msg_1_field();
  inline void set_has_test_msg_2_field();
  inline void clear_has_test_msg_2_field();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::crier::test::test_msg_1* test_msg_1_field_;
  ::crier::test::test_msg_2* test_msg_2_field_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  friend void  protobuf_AddDesc_CrierTest_2eproto();
  friend void protobuf_AssignDesc_CrierTest_2eproto();
  friend void protobuf_ShutdownFile_CrierTest_2eproto();

  void InitAsDefaultInstance();
  static root_msg* default_instance_;
};
// ===================================================================


// ===================================================================

// test_msg_1

// required uint32 id = 1;
inline bool test_msg_1::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void test_msg_1::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void test_msg_1::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void test_msg_1::clear_id() {
  id_ = 0u;
  clear_has_id();
}
inline ::google::protobuf::uint32 test_msg_1::id() const {
  return id_;
}
inline void test_msg_1::set_id(::google::protobuf::uint32 value) {
  set_has_id();
  id_ = value;
}

// -------------------------------------------------------------------

// test_msg_2

// required string data = 1;
inline bool test_msg_2::has_data() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void test_msg_2::set_has_data() {
  _has_bits_[0] |= 0x00000001u;
}
inline void test_msg_2::clear_has_data() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void test_msg_2::clear_data() {
  if (data_ != &::google::protobuf::internal::kEmptyString) {
    data_->clear();
  }
  clear_has_data();
}
inline const ::std::string& test_msg_2::data() const {
  return *data_;
}
inline void test_msg_2::set_data(const ::std::string& value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(value);
}
inline void test_msg_2::set_data(const char* value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(value);
}
inline void test_msg_2::set_data(const char* value, size_t size) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* test_msg_2::mutable_data() {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  return data_;
}
inline ::std::string* test_msg_2::release_data() {
  clear_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = data_;
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void test_msg_2::set_allocated_data(::std::string* data) {
  if (data_ != &::google::protobuf::internal::kEmptyString) {
    delete data_;
  }
  if (data) {
    set_has_data();
    data_ = data;
  } else {
    clear_has_data();
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// -------------------------------------------------------------------

// root_msg

// optional .crier.test.test_msg_1 test_msg_1_field = 1;
inline bool root_msg::has_test_msg_1_field() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void root_msg::set_has_test_msg_1_field() {
  _has_bits_[0] |= 0x00000001u;
}
inline void root_msg::clear_has_test_msg_1_field() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void root_msg::clear_test_msg_1_field() {
  if (test_msg_1_field_ != NULL) test_msg_1_field_->::crier::test::test_msg_1::Clear();
  clear_has_test_msg_1_field();
}
inline const ::crier::test::test_msg_1& root_msg::test_msg_1_field() const {
  return test_msg_1_field_ != NULL ? *test_msg_1_field_ : *default_instance_->test_msg_1_field_;
}
inline ::crier::test::test_msg_1* root_msg::mutable_test_msg_1_field() {
  set_has_test_msg_1_field();
  if (test_msg_1_field_ == NULL) test_msg_1_field_ = new ::crier::test::test_msg_1;
  return test_msg_1_field_;
}
inline ::crier::test::test_msg_1* root_msg::release_test_msg_1_field() {
  clear_has_test_msg_1_field();
  ::crier::test::test_msg_1* temp = test_msg_1_field_;
  test_msg_1_field_ = NULL;
  return temp;
}
inline void root_msg::set_allocated_test_msg_1_field(::crier::test::test_msg_1* test_msg_1_field) {
  delete test_msg_1_field_;
  test_msg_1_field_ = test_msg_1_field;
  if (test_msg_1_field) {
    set_has_test_msg_1_field();
  } else {
    clear_has_test_msg_1_field();
  }
}

// optional .crier.test.test_msg_2 test_msg_2_field = 2;
inline bool root_msg::has_test_msg_2_field() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void root_msg::set_has_test_msg_2_field() {
  _has_bits_[0] |= 0x00000002u;
}
inline void root_msg::clear_has_test_msg_2_field() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void root_msg::clear_test_msg_2_field() {
  if (test_msg_2_field_ != NULL) test_msg_2_field_->::crier::test::test_msg_2::Clear();
  clear_has_test_msg_2_field();
}
inline const ::crier::test::test_msg_2& root_msg::test_msg_2_field() const {
  return test_msg_2_field_ != NULL ? *test_msg_2_field_ : *default_instance_->test_msg_2_field_;
}
inline ::crier::test::test_msg_2* root_msg::mutable_test_msg_2_field() {
  set_has_test_msg_2_field();
  if (test_msg_2_field_ == NULL) test_msg_2_field_ = new ::crier::test::test_msg_2;
  return test_msg_2_field_;
}
inline ::crier::test::test_msg_2* root_msg::release_test_msg_2_field() {
  clear_has_test_msg_2_field();
  ::crier::test::test_msg_2* temp = test_msg_2_field_;
  test_msg_2_field_ = NULL;
  return temp;
}
inline void root_msg::set_allocated_test_msg_2_field(::crier::test::test_msg_2* test_msg_2_field) {
  delete test_msg_2_field_;
  test_msg_2_field_ = test_msg_2_field;
  if (test_msg_2_field) {
    set_has_test_msg_2_field();
  } else {
    clear_has_test_msg_2_field();
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace test
}  // namespace crier

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_CrierTest_2eproto__INCLUDED
