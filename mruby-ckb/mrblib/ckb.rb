module CKB
  module Source
    CURRENT = 0
    INPUT = 1
    OUTPUT = 2
    DEP = 3
  end

  class Reader
    def exists?
      internal_read(1, 0) != nil
    end

    def length
      internal_read(0)
    end

    def read(offset, len)
      internal_read(len, offset)
    end

    def readall
      read(0, length)
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
    LOCK_HASH = 3
    TYPE = 4
    TYPE_HASH = 5
    LOCK = 6

    def initialize(source, index, cell_field)
      @source = source
      @index = index
      @field = cell_field
    end
  end

  class InputField < Reader
    ARGS = 0
    OUT_POINT = 1

    def initialize(source, index, input_field)
      @source = source
      @index = index
      @field = input_field
    end
  end

  module HashType
    LOCK = CellField::LOCK_HASH
    TYPE = CellField::TYPE_HASH
  end
end
