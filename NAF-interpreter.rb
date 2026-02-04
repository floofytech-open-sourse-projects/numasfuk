# PART 1 / 2

class Numasfuk
  MAX_STRINGS = 16
  STR_SIZE    = 64

  def initialize
    @strings = Array.new(MAX_STRINGS) { "" }
    @current = 0
    @reg     = 0   # numeric register
  end

  def run(code)
    i = 0
    while i < code.length
      case code[i]
      when "1","2","3","7"
        dispatch_single(code[i])
        i += 1
      when "4"
        read_input
        i += 1
      when "5"
        i = print_hex(code, i) + 1
      when "6"
        i = repeat(code, i) + 1
      when "8"
        i = print_binary(code, i) + 1
      when "9"
        dump
        return

      # v2 extensions
      when "A" then tape_forward;       i += 1
      when "B" then tape_back;          i += 1
      when "E" then empty_check;        i += 1
      when "J" then i = jump_abs(code, i)
      when "Z" then i = jump_if_zero(code, i)
      when "N" then i = jump_if_nonzero(code, i)
      when "0" then i = set_reg(code, i) + 1
      when "+" then i = add_reg(code, i) + 1
      when "-" then i = sub_reg(code, i) + 1
      when "C" then i = compare_hex(code, i) + 1

      else
        i += 1
      end
    end
  end

  private

  def dispatch_single(op)
    case op
    when "1" then append_one
    when "2" then new_string
    when "3" then to_float_string
    when "7" then random_digit
    end
  end

  def current_str
    @strings[@current]
  end

  def current_str=(val)
    @strings[@current] = val.to_s[0, STR_SIZE]
  end

  # base ops
  def append_one
    return if current_str.length >= STR_SIZE
    self.current_str = current_str + "1"
  end

  def new_string
    return if @current >= MAX_STRINGS - 1
    @current += 1
    @strings[@current] = ""
  end

  def to_float_string
    self.current_str = "0.0"
  end

  def read_input
    line = STDIN.gets
    return if line.nil?
    self.current_str = line.chomp[0, STR_SIZE]
  end

  def random_digit
    self.current_str = rand(10).to_s
  end

  # tape movement
  def tape_forward
    @current = (@current + 1) % MAX_STRINGS
  end

  def tape_back
    @current = (@current - 1) % MAX_STRINGS
  end

  # numeric register
  def set_reg(code, i)
    val, j = parse_paren_int(code, i + 1)
    @reg = val
    j
  end

  def add_reg(code, i)
    val, j = parse_paren_int(code, i + 1)
    @reg += val
    j
  end

  def sub_reg(code, i)
    val, j = parse_paren_int(code, i + 1)
    @reg -= val
    j
  end
# PART 2 / 2

  # jumps
  def jump_abs(code, i)
    pos, j = parse_paren_int(code, i + 1)
    pos
  end

  def jump_if_zero(code, i)
    pos, j = parse_paren_int(code, i + 1)
    @reg == 0 ? pos : j + 1
  end

  def jump_if_nonzero(code, i)
    pos, j = parse_paren_int(code, i + 1)
    @reg != 0 ? pos : j + 1
  end

  # comparisons
  def empty_check
    @reg = current_str.empty? ? 0 : 1
  end

  def compare_hex(code, i)
    bytes, j = parse_paren_hex(code, i + 1)
    target = bytes.first
    first  = current_str.bytes.first || 0
    @reg = (first == target) ? 0 : 1
    j
  end

  # printing ops
  def print_hex(code, i)
    bytes, j = parse_paren_hex(code, i + 1)
    STDOUT.write(bytes.pack("C*"))
    STDOUT.flush
    j
  end

  def repeat(code, i)
    n, j = parse_paren_int(code, i + 1)
    op_index = j + 1
    return j if op_index >= code.length

    op = code[op_index]
    # if not repeatable, just fall through so X runs once normally
    return j unless "1237".include?(op)

    n.times { dispatch_single(op) }
    # caller adds +1, so we continue after X
    op_index
  end

  def print_binary(code, i)
    val, j = parse_paren_bin(code, i + 1)
    STDOUT.write(val.chr(Encoding::BINARY))
    STDOUT.flush
    j
  end

  def dump
    @strings[0..@current].each { |s| puts s }
  end

  # parsing helpers
  def parse_paren_int(code, i)
    i = skip_until(code, i, "(")
    i += 1
    num = 0
    while i < code.length && code[i] != ")"
      c = code[i]
      break unless ("0".."9").include?(c)
      num = num * 10 + (c.ord - 48)
      i += 1
    end
    [num, i]
  end

  def parse_paren_hex(code, i)
    i = skip_until(code, i, "(")
    i += 1
    bytes = []
    buf = ""
    while i < code.length && code[i] != ")"
      c = code[i]
      if c =~ /[0-9A-Fa-f]/
        buf << c
        if buf.length == 2
          bytes << buf.to_i(16)
          buf.clear
        end
      end
      i += 1
    end
    [bytes, i]
  end

  def parse_paren_bin(code, i)
    i = skip_until(code, i, "(")
    i += 1
    val = 0
    while i < code.length && code[i] != ")"
      c = code[i]
      val = (val << 1) | (c == "1" ? 1 : 0)
      i += 1
    end
    [val & 0xFF, i]
  end

  def skip_until(code, i, ch)
    i += 1 while i < code.length && code[i] != ch
    i
  end
end

if __FILE__ == $0
  abort "usage: #{$0} file.naf" unless ARGV.size == 1
  code = File.read(ARGV[0])
  Numasfuk.new.run(code)
end

