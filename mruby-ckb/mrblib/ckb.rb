module CKB
  module Source
    INPUT = 0
    OUTPUT = 1
    CURRENT = 2
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
    LOCK_HASH = 2
    CONTRACT = 3
    CONTRACT_HASH = 4

    def initialize(source, index, cell_field)
      @source = source
      @index = index
      @field = cell_field
    end
  end

  class InputField < Reader
    UNLOCK = 0
    OUT_POINT = 1

    def initialize(source, index, input_field)
      @source = source
      @index = index
      @field = input_field
    end
  end

  module Category
    LOCK = CellField::LOCK_HASH
    CONTRACT = CellField::CONTRACT_HASH
  end
end
