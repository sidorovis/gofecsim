package main

import "fmt"
import "math/rand"
import "math"
import "runtime"
import "os"
import "time"

type Network struct {
    rtt int64
    loss_pr float64 // percentage (1.0 == 100%)
    bandwidth float64
    identifier string
}

func (this Network) received() bool {
    return rand.Float64() >= this.loss_pr
}


type FecModel struct {
    packetsPerKeyFrame float64
    packetsPerDeltaFrame float64
    fec int64
    network Network
}

func (this FecModel) frame_was_received(required_packets int64, fec float64) (bool) {
    packets := int64(float64(required_packets) * (1.0 + fec) + 0.5)
    received_packets := int64(0)
    for p := int64(0) ; p < packets ; p++ {
	if (this.network.received()) {
	    received_packets++
	}
    }
    return received_packets >= required_packets;
}

func (this FecModel) findKD() (float64, float64, float64, float64) {

    k_bf_values := [32] float64 { 0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.07, 0.087, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29, 0.3, 0.31, 0.32, 0.33, 0.34, 0.35, 0.38, 0.4, 0.42, 0.45, 0.50, 0.67, 0.8, 0.9, 1.0 } // percentage (1.0 == 100%), key frames FEC protection amount, for 5 packets will send 5 * (1+k) packets
    d_bf_values := [32] float64 { 0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.07, 0.087, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29, 0.3, 0.31, 0.32, 0.33, 0.34, 0.35, 0.38, 0.4, 0.42, 0.45, 0.50, 0.67, 0.8, 0.9, 1.0 } // percentage (1.0 == 100%), delta frames FEC protection amount, for 5 packets will send 5 * (1+d) packets

    best_k := float64(0.0)
    best_d := float64(0.0)
    best_loss_k := float64(100.0)
    best_loss_d := float64(100.0)
    best_rating := math.MaxFloat64

    video_length := 5 * 60 * this.fec
    key_frame_sparce := 17
    frame_rate := float64(1000.0) / float64(this.fec)

    for _, k := range k_bf_values {
	for _, d := range d_bf_values {
		total_k := float64(0)
		loss_k := float64(0)
		total_d := float64(0)
		loss_d := float64(0)

		for frame_index := int64(0); frame_index < video_length ; frame_index++ {
			if (math.Mod(float64(frame_index), float64(key_frame_sparce)) == 0) {
				// key frame
				packets := int64(this.packetsPerKeyFrame + 0.5)
				total_k++
				if (!this.frame_was_received(packets, k)) {
					loss_k = loss_k + 1
				}
			} else {
				// delta frame
				packets := int64(this.packetsPerDeltaFrame + 0.5)
				total_d = total_d + 1
				if (!this.frame_was_received(packets, d)) {
					loss_d = loss_d + 1
				}
			}
		}
		
		if (total_k > 0.0 && total_d > 0.0) {
			loss_k_percent := loss_k / total_k
			loss_d_percent := loss_d / total_d
			rating := this.calculate_rating(frame_rate, loss_k, loss_d, loss_d_percent, k, d)
			if (best_rating > rating) {
				best_rating = rating
				best_loss_k = loss_k_percent
				best_loss_d = loss_d_percent
				best_k = k
				best_d = d
			}
		}
	}
    }

    return best_k, best_d, best_loss_k, best_loss_d
}

func (this FecModel) calculate_rating(frame_rate float64, loss_k float64, loss_d float64, loss_d_percent float64, k float64, d float64) (float64) {
    if (float64(this.network.rtt) < frame_rate && loss_d_percent < 0.05) {
	loss_d = 0.0
    }
    return float64(0.0) + 20.0 * loss_k + 4 * loss_d + 1.5 * k + d;
}

type SimulateResult struct {
    best_k float64
    best_d float64
    loss_k float64
    loss_d float64
}

func simulate(rtt int64, loss_pr float64, packetsPerK float64, packetsPerD float64, resultChan chan SimulateResult) {
	network := Network{ rtt, loss_pr, 32000.0, "naive model"}
	fec_model := FecModel{ packetsPerK, packetsPerD, 15, network }
	best_k, best_d, loss_k, loss_d := fec_model.findKD()
	result := SimulateResult{ best_k, best_d, loss_k, loss_d }
	resultChan <- result
}


func main() {
    runtime.GOMAXPROCS(8)

    rtt_bf_values := [1] int64 { 30 }
    loss_pr_bf_values := [31] float64 { 0.0, 0.011, 0.016, 0.023, 0.029, 0.033, 0.039, 0.043, 0.047, 0.056, 0.066, 0.071, 0.078, 0.083, 0.092, 0.1, 0.12, 0.16, 0.19, 0.22, 0.24, 0.26, 0.29, 0.34, 0.38, 0.43, 0.49, 0.57, 0.8, 0.9, 1.0 }

    f, err := os.Create("out.txt")
    if (err != nil) {
	fmt.Printf("File cannot be opened for writing")
	return
    }
    defer f.Close()

    for _, rtt := range rtt_bf_values {
	for _, loss_pr := range loss_pr_bf_values {
		var resultChan chan SimulateResult = make(chan SimulateResult)
		simulations := int64(8)
		for i := int64(0) ; i < simulations ; i++ {
		    go simulate(rtt, loss_pr, 11.5, 4.54, resultChan)
		}
		summ_sr := SimulateResult{ 0.0, 0.0, 0.0, 0.0 }
		actual_results := int64(0)
		for i := int64(0) ; i < simulations ; i++ {
		    sr := <- resultChan
		    summ_sr.best_k += sr.best_k
		    summ_sr.best_d += sr.best_d
		    summ_sr.loss_k += sr.loss_k
		    summ_sr.loss_d += sr.loss_d
		    actual_results += 1
		}
		
		summ_sr.best_k /= float64(simulations)
		summ_sr.best_d /= float64(simulations)
		summ_sr.loss_k /= float64(simulations)
		summ_sr.loss_d /= float64(simulations)
		
		t := time.Now()
		fmt.Fprintf(f, "%s | rtt %d\tloss_pr %f\t --> BestK: %f\tBestD: %f\t(LossK: %f, LossD:%f) ", 
			t.String(), rtt, loss_pr, summ_sr.best_k, summ_sr.best_d, summ_sr.loss_k, summ_sr.loss_d )
			
		if (actual_results != simulations) {
		    fmt.Fprintf(f, " Not all results were acquired\n")
		} else {
		    fmt.Fprintf(f, "\n")
		}
		f.Sync()
	}
    }

}
