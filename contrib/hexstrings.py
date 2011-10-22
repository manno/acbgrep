import sys
import re
'''
encode input ascii sequence to different representations
'''
class PatternBuilder:

  '''
  bcd: 4 bits each digit, only works for numbers
  '''
  @classmethod
  def createBCD(cls, string):
    bcd = ""
    i=0
    while(i<len(string)):
      r = (ord(string[i])-0x30) << 4
      r = r | ord(string[i+1])-0x30
      i+=2
      bcd+=chr(r)
    return bcd

  '''
  unicode 16, only works for ascii chars :)
  '''
  @classmethod
  def createUnicode(cls, string):
    return "\x00" + "\x00".join(list(string))

  '''
  TODO EBCDIC
  '''

  @classmethod
  def create(cls, p, string):    
    p.append( string.encode("hex") )
    p.append( cls.createUnicode( string ).encode("hex") )
    if (len(string)%2 == 0) and (re.search("^[0-9]+$", string)):
      p.append( cls.createBCD( string ).encode("hex") )
    else:
      # TODO maybe zero pad
      print "WARNING: can't create bcd"

def main():
  if len(sys.argv) < 2:
    sys.exit('Usage: %s pattern1 pattern2' % sys.argv[0])

  p = list()
  for arg in sys.argv[1:]:
      PatternBuilder().create(p, arg)
  for pattern in p:
    print pattern

main()
