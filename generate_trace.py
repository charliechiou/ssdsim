import random
import sys


def generate_trace():
    print("SSD Simulator Trace Generator")
    print("=============================")

    try:
        filename = input("Output filename [default: trace.ascii]: ").strip()
        if not filename:
            filename = "trace.ascii"

        num_requests = input("Number of requests [default: 1000]: ").strip()
        num_requests = int(num_requests) if num_requests else 1000

        read_ratio = input("Read percentage (0-100) [default: 50]: ").strip()
        read_ratio = float(read_ratio) if read_ratio else 50.0
        read_prob = read_ratio / 100.0

        min_lsn = input("Min LSN [default: 0]: ").strip()
        min_lsn = int(min_lsn) if min_lsn else 0

        max_lsn = input("Max LSN [default: 100000]: ").strip()
        max_lsn = int(max_lsn) if max_lsn else 100000

        avg_size = input(
            "Average request size (sectors) [default: 8]: ").strip()
        avg_size = int(avg_size) if avg_size else 8

        avg_interval = input(
            "Average inter-arrival time (ns) [default: 100000]: ").strip()
        avg_interval = int(avg_interval) if avg_interval else 100000

        device_id = 0
        current_time = 0

        print(f"\nGenerating {num_requests} requests to {filename}...")

        with open(filename, 'w') as f:
            for _ in range(num_requests):
                # Time
                interval = int(random.expovariate(1.0 / avg_interval))
                current_time += interval

                # LSN
                lsn = random.randint(min_lsn, max_lsn)

                # Size (simple random variation around average, min 1)
                size = max(1, int(random.gauss(avg_size, avg_size/4)))

                # Operation (1 = Read, 0 = Write)
                ope = 1 if random.random() < read_prob else 0

                # Format: time_t device lsn size ope
                line = f"{current_time} {device_id} {lsn} {size} {ope}\n"
                f.write(line)

        print("Done!")

    except ValueError as e:
        print(f"Error: Invalid input - {e}")
    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    generate_trace()
