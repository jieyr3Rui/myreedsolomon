//go:build ignore
// +build ignore

// Copyright 2015, Klaus Post, see LICENSE for details.
//
// Stream decoder example.
//
// The decoder reverses the process of "stream-encoder.go"
//
// To build an executable use:
//
// go build stream-decoder.go
//
// Simple Encoder/Decoder Shortcomings:
// * If the file size of the input isn't dividable by the number of data shards
//   the output will contain extra zeroes
//
// * If the shard numbers isn't the same for the decoder as in the
//   encoder, invalid output will be generated.
//
// * If values have changed in a shard, it cannot be reconstructed.
//
// * If two shards have been swapped, reconstruction will always fail.
//   You need to supply the shards in the same order as they were given to you.
//
// The solution for this is to save a metadata file containing:
//
// * File size.
// * The number of data/parity shards.
// * HASH of each shard.
// * Order of the shards.
//
// If you save these properties, you should abe able to detect file corruption
// in a shard and be able to reconstruct your data if you have the needed number of shards left.

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"github.com/klauspost/reedsolomon"
)

var inFile = flag.String("in", "", "Input shards name")
var dataShards = flag.Int("data", 4, "Number of shards to split the data into")
var parShards = flag.Int("par", 2, "Number of parity shards")
var outDir = flag.String("out", "./recover", "Alternative output path")

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "Usage of %s:\n", os.Args[0])
		fmt.Fprintf(os.Stderr, "  %s [-flags] basefile.ext\nDo not add the number to the filename.\n", os.Args[0])
		fmt.Fprintf(os.Stderr, "Valid flags:\n")
		flag.PrintDefaults()
	}
}

func main() {
	// Parse flags
	flag.Parse()
	fname := *inFile

	// Create matrix
	enc, err := reedsolomon.NewStream(*dataShards, *parShards)
	checkErr(err)

	fmt.Println("\nTry to open shards ...")
	// Open the inputs
	shards, size, err := openInput(*dataShards, *parShards, fname)
	checkErr(err)

	// Verify the shards
	ok, err := enc.Verify(shards)
	if ok {
		fmt.Println("\nNo reconstruction needed")
	} else {
		fmt.Println("\nVerification failed. Reconstructing data ...")
		shards, size, err = openInput(*dataShards, *parShards, fname)
		checkErr(err)
		// Create out destination writers
		out := make([]io.Writer, len(shards))
		for i := range out {
			// 如果这个shards没有就造一个
			if shards[i] == nil {
				outfn := fmt.Sprintf("%s.%d", fname, i)
				fmt.Println("Creating", outfn)
				out[i], err = os.Create(outfn)
				checkErr(err)
			}
		}
		// Reconstruct will recreate the missing shards if possible.
		err = enc.Reconstruct(shards, out)
		if err != nil {
			fmt.Println("Reconstruct failed -", err)
			os.Exit(1)
		}
		// Close output.
		for i := range out {
			if out[i] != nil {
				err := out[i].(*os.File).Close()
				checkErr(err)
			}
		}
		fmt.Println("Reconstruct finished!")

		// 验证shards
		fmt.Println("\nVerifiing shards ...")
		shards, size, err = openInput(*dataShards, *parShards, fname)
		ok, err = enc.Verify(shards)
		if !ok {
			fmt.Println("\nVerification failed after reconstruction, data likely corrupted:", err)
			os.Exit(1)
		}
		checkErr(err)
		fmt.Println("Verification finished!")
	}

	// Join the shards and write them
	// outfn := *outFile
	// if outfn == "" {
	// 	outfn = fname
	// }

	// Join the shards and write them
	dir, file := filepath.Split(fname)
	if *outDir != "" {
		dir = *outDir
	}
	outfn := filepath.Join(dir, file)

	fmt.Println("\nWriting data to", outfn)
	f, err := os.Create(outfn)
	checkErr(err)

	shards, size, err = openInput(*dataShards, *parShards, fname)
	checkErr(err)

	// We don't know the exact filesize.
	err = enc.Join(f, shards, int64(*dataShards)*size)
	checkErr(err)
}

/*
	尝试打开shards
*/
func openInput(dataShards, parShards int, fname string) (r []io.Reader, size int64, err error) {
	// Create shards and load the data.
	shards := make([]io.Reader, dataShards+parShards)
	for i := range shards {
		infn := fmt.Sprintf("%s.%d", fname, i)
		fmt.Println("Opening", infn)
		f, err := os.Open(infn)
		if err != nil {
			fmt.Println("Error reading file", err)
			shards[i] = nil
			continue
		} else {
			shards[i] = f
		}
		stat, err := f.Stat()
		checkErr(err)
		if stat.Size() > 0 {
			size = stat.Size()
		} else {
			shards[i] = nil
		}
	}
	return shards, size, nil
}

func checkErr(err error) {
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %s", err.Error())
		os.Exit(2)
	}
}
