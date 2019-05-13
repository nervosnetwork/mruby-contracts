module CKB
  class IndexOutOfBound < ::StandardError
  end

  module Source
    INPUT = 1
    OUTPUT = 2
    DEP = 3
  end

  class Reader
    def exists?
      !!length
    end

    def length
      internal_read(0)
    end

    def read(offset, len)
      internal_read(len, offset)
    end

    def readall
      # This way we can save one extra syscall
      l = length
      return nil unless l
      read(0, l)
    end
  end

  class Cell < Reader
    def initialize(source, index)
      @source = source
      @index = index
    end
  end

  class CellField < Reader
    CAPACITY = 0
    DATA = 1
    DATA_HASH = 2
    LOCK = 3
    LOCK_HASH = 4
    TYPE = 5
    TYPE_HASH = 6

    def initialize(source, index, cell_field)
      @source = source
      @index = index
      @field = cell_field
    end
  end

  class InputField < Reader
    ARGS = 0
    OUT_POINT = 1
    SINCE = 2

    def initialize(source, index, input_field)
      @source = source
      @index = index
      @field = input_field
    end
  end

  module HashType
    LOCK_HASH = CellField::LOCK_HASH
    TYPE_HASH = CellField::TYPE_HASH
  end
end
