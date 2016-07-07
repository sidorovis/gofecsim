package main

import "fmt"
import "math/rand"
import "math"

type Network struct {
    rtt int64
    loss_pr float64 // percentage (1.0 == 100%)
    bandwidth float64
    identifier string
}

func (this Network) received() bool {
    return rand.Float64() > this.loss_pr
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

    k_bf_values := [26] float64 { 0, 0.03, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29, 0.3, 0.31, 0.32, 0.33, 0.34, 0.35, 0.38, 0.4, 0.42, 0.45, 0.50, 0.67, 0.8, 0.9, 1.0 } // percentage (1.0 == 100%), key frames FEC protection amount, for 5 packets will send 5 * (1+k) packets
    d_bf_values := [26] float64 { 0, 0.03, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29, 0.3, 0.31, 0.32, 0.33, 0.34, 0.35, 0.38, 0.4, 0.42, 0.45, 0.50, 0.67, 0.8, 0.9, 1.0 } // percentage (1.0 == 100%), delta frames FEC protection amount, for 5 packets will send 5 * (1+d) packets

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
				if (this.frame_was_received(packets, k)) {
					loss_k = loss_k + 1
				}
			} else {
				// delta frame
				packets := int64(this.packetsPerDeltaFrame + 0.5)
				total_d = total_d + 1
				if (this.frame_was_received(packets, d)) {
					loss_d = loss_d + 1
				}
			}
		}
		
		if (total_k > 0.0 && total_d > 0.0) {
			loss_k_percent := loss_k / total_k
			loss_d_percent := loss_d / total_d
			rating := this.calculate_rating(frame_rate, loss_k_percent, loss_d_percent, k, d)
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

func (this FecModel) calculate_rating(frame_rate float64, loss_k float64, loss_d float64, k float64, d float64) (float64) {
    if (float64(this.network.rtt) < frame_rate && loss_d < 0.05) {
	loss_d = 0.05
    }
    return float64(1.0) + 20.0 * loss_k + 4 * loss_d + 1.5 * k + d;
}


func main() {
    rtt_bf_values := [1] int64 { 30 }
    loss_pr_bf_values := [1] float64 { 0.25 }

    for _, rtt := range rtt_bf_values {
	for _, loss_pr := range loss_pr_bf_values {
		
	
		network := Network{ rtt, loss_pr, 32000.0, "naive model"}
		fec_model := FecModel{ 11.5, 4.54, 15, network }
		best_k, best_d, loss_k, loss_d := fec_model.findKD()
		fmt.Printf("BestK: %f\tBestD: %f\t(LossK: %f, LossD:%f) \n", best_k, best_d, loss_k, loss_d )
	}
    }

}
