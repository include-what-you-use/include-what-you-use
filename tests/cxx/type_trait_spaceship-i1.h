
class Foo {};
class Bar {};

int operator<=>(const Foo& a, const Bar& b) {
  return -1;
}
