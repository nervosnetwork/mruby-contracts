module CKB
  INPUT = 0
  OUTPUT = 1

  class Cell
    def initialize(source, index)
      raise "Invalid source" if source != INPUT && source != OUTPUT
      @source = source
      @index = index.to_i
    end
  end
end
