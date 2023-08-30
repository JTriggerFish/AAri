import AAri_cpp  # Import the Pybind11 module
import time
import re


class AudioEngineWrapper:
    def __init__(self):
        self.engine = AAri.AudioEngine()

    def start(self):
        self.engine.startAudio()

    def stop(self):
        self.engine.stopAudio()


class AudioGraphWrapper:
    def __init__(self, engine_wrapper):
        self.engine_wrapper = engine_wrapper
        self.graph = self.engine_wrapper.engine.get_graph()
        self._immutable_blocks = set()  # To keep track of blocks that have been added to the graph

    def add_block(self, block):
        if block in self._immutable_blocks:
            raise ValueError("This block has already been added to the graph and is immutable.")
        block_ptr = self.graph.add_block(block.block_ptr)
        self._immutable_blocks.add(block)
        return block_ptr


class BlockWrapper:
    def __init__(self, block_ptr):
        self.block_ptr = block_ptr

    def __rshift__(self, other):
        # Implement parsing logic for advanced connections
        # Placeholder: Parse and print the connection strings for demonstration
        connection_type = "default"  # Default connection type
        if isinstance(other, tuple):
            connection_type = other[1]
            other = other[0]
        print(f"Advanced connection from {self} to {other} with type {connection_type}")
        return self

    def __mul__(self, other):
        # Implement parsing logic for amplitude and other settings
        # Placeholder: Parse and print the multiplication strings for demonstration
        if isinstance(other, str):
            if "db" in other:
                print(f"Setting amplitude of {self} to {other}")
            elif "+" in other:
                print(f"Modifying {self} with {other}")
        return self


class MonoToStereoWrapper(BlockWrapper):
    def __init__(self, amp_db=-30.0, panning=0.0):
        super().__init__(AAri.MonoToStereo(amp_db, panning))


class SineOscWrapper(BlockWrapper):
    def __init__(self, freq=440.0, amplitude=1.0):
        super().__init__(AAri.SineOsc(freq, amplitude))


# Testing the wrappers
if __name__ == "__main__":
    engine = AudioEngineWrapper()
    graph = AudioGraphWrapper(engine)
    engine.start()

    # Create some blocks and add them to the graph
    osc1 = SineOscWrapper(440.0, 1.0)
    graph.add_block(osc1)
    mixer = MonoToStereoWrapper()
    graph.add_block(mixer)

    # Test the advanced features
    osc1 >> mixer
    osc1 >> (mixer, "FREQ")
    "110.0 + 0.1" * osc1 >> mixer
    "-30db" * osc1 >> mixer

    time.sleep(3)  # Let the engine run for 3 seconds
    engine.stop()
